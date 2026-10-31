import { watch } from "node:fs";
import { cp, mkdir, mkdtemp, rename, rm } from "node:fs/promises";
import { networkInterfaces } from "node:os";
import path from "node:path";
import { parseArgs } from "node:util";
import { corePlugin, createHostClient, type ImplementedHostPlugin } from "@vitadeck/sdk/host-control";
import { DEFAULT_CONFIG_PATH, installedPluginsDir, isPort, loadConfig, readConfig } from "./config";
import { linkVita } from "./link";
import {
  HOST_PLUGIN_ARCHIVE_EXTENSION,
  HOST_PLUGIN_EXTENSION,
  extractHostPluginArchive,
  hostPluginPackagesIn,
  isHostPluginId,
  loadHostPluginPackage,
} from "./plugin";
import { startHostCompanion } from "./server";
import { systemStats } from "./system-stats";

const USAGE = `Usage: vitadeck-host-companion <command>

  start [--port N] [--plugin <name.vdplugin|dir>]... [--config <path>]
      Serve Host Control commands on 0.0.0.0, port 8797 by default. Loads the built-in plugins, the installed
      ones, and each --plugin package or every .vdplugin package in a --plugin folder, and reloads them when
      they change.
  link <vita-address>[:port] [--port N] [--advertise <ip>] [--config <path>]
      Link the VitaDeck showing that address (port 8787 by default) with the running companion.
  install <name.vdplugin.zip|name.vdplugin> [--config <path>]
      Install a Host Plugin Package that a Deck App's author built. It runs with your permissions on this
      computer.
  remove <plugin-id> [--config <path>]
      Uninstall a plugin.
  notify <text> [--port N] [--config <path>]
      Send a notice to every Deck App watching this computer, e.g. after a build: make && vitadeck-host-companion notify "Build done".`;

const BUILT_IN_PLUGINS: readonly ImplementedHostPlugin[] = [systemStats];
const BUILT_IN_IDS = new Set(["core", ...BUILT_IN_PLUGINS.map((plugin) => plugin.id)]);
const RELOAD_QUIET_MS = 200;

// `pnpm --dir js host-companion` runs from js/, so paths are relative to INIT_CWD, where the user typed it. Anywhere
// else, a Deck App's `host` script included, they are relative to the working directory.
const userCwd =
  process.env.npm_lifecycle_event === "host-companion" ? (process.env.INIT_CWD ?? process.cwd()) : process.cwd();

function parsePort(value: string | undefined): number | undefined {
  if (value === undefined) return undefined;
  const port = Number(value);
  if (!isPort(port)) throw new Error(`--port must be a number from 1 to 65535, not ${value}.`);
  return port;
}

function lanUrls(port: number): string[] {
  const addresses = Object.values(networkInterfaces())
    .flatMap((list) => list ?? [])
    .filter((info) => info.family === "IPv4" && !info.internal)
    .map((info) => `http://${info.address}:${port}/`);
  return addresses.length > 0 ? addresses : [`http://127.0.0.1:${port}/`];
}

function watchFailed(dir: string) {
  return (error: Error) => {
    console.error(`Stopped watching ${dir} for plugin changes: ${error.message}`);
  };
}

function describe(plugins: readonly ImplementedHostPlugin[]): string {
  return ["core", ...plugins.map((plugin) => plugin.id)].join(", ");
}

async function start(values: { port?: string; plugin?: string[] }, configPath: string): Promise<void> {
  const config = await loadConfig(configPath);
  const pluginsDir = installedPluginsDir(configPath);
  await mkdir(pluginsDir, { recursive: true });
  const extra = (values.plugin ?? []).map((given) => path.resolve(userCwd, given));
  const archive = extra.find((entry) => entry.endsWith(HOST_PLUGIN_ARCHIVE_EXTENSION));
  if (archive) throw new Error(`--plugin takes a ${HOST_PLUGIN_EXTENSION} folder; install ${archive} instead.`);
  // A path ending in .vdplugin is one package; any other path is a folder of them, like a Deck App's dist/.
  const extraPackages = extra.filter((entry) => entry.endsWith(HOST_PLUGIN_EXTENSION));
  const extraDirs = extra.filter((entry) => !entry.endsWith(HOST_PLUGIN_EXTENSION));
  const loadPlugins = async () => {
    const listed = await Promise.all([pluginsDir, ...extraDirs].map((dir) => hostPluginPackagesIn(dir)));
    // A --plugin package replaces an installed plugin with the same id, so an author can work on one they installed.
    const loaded = await Promise.all([...listed.flat(), ...extraPackages].map((dir) => loadHostPluginPackage(dir)));
    const custom = new Map(loaded.map((plugin) => [plugin.id, plugin]));
    return [...BUILT_IN_PLUGINS, ...custom.values()];
  };

  const plugins = await loadPlugins();
  const companion = await startHostCompanion({
    port: parsePort(values.port) ?? config.port,
    name: config.name,
    plugins,
    vitas: () =>
      readConfig(configPath).then(
        (current) => current.config.vitas,
        () => config.vitas,
      ),
  });
  console.log(`Host Companion "${config.name}" is listening on ${lanUrls(companion.port).join(", ")}`);
  console.log(`Plugins: ${describe(plugins)}`);
  console.log(
    config.vitas.length > 0
      ? `Linked Vitas: ${config.vitas.join(", ")}`
      : "Link a Vita with: vitadeck-host-companion link <vita-address> (the address under Upload and link in the VitaDeck Shell)",
  );

  // Parent folders, recursively: a rebuild or reinstall replaces the package folder, which ends a watch on it.
  let timer: NodeJS.Timeout | undefined;
  const reload = () => {
    clearTimeout(timer);
    timer = setTimeout(() => {
      void (async () => {
        try {
          const next = await loadPlugins();
          companion.setPlugins(next);
          console.log(`Reloaded plugins: ${describe(next)}`);
        } catch (error) {
          console.error(`Kept the previous plugins: ${error instanceof Error ? error.message : String(error)}`);
        }
      })();
    }, RELOAD_QUIET_MS);
  };
  const watched = [
    ...[pluginsDir, ...extraDirs].map((dir) => ({ dir, prefix: "" })),
    ...extraPackages.map((entry) => ({ dir: path.dirname(entry), prefix: path.basename(entry) })),
  ];
  for (const { dir, prefix } of watched) {
    watch(dir, { recursive: true }, (_event, name) => {
      if (name !== null && name.startsWith(prefix) && name.includes(HOST_PLUGIN_EXTENSION)) reload();
    }).on("error", watchFailed(dir));
  }
}

async function install(given: string, configPath: string): Promise<void> {
  const source = path.resolve(userCwd, given);
  if (!source.endsWith(HOST_PLUGIN_ARCHIVE_EXTENSION) && !source.endsWith(HOST_PLUGIN_EXTENSION)) {
    throw new Error(`${given} is not a ${HOST_PLUGIN_ARCHIVE_EXTENSION} archive or a ${HOST_PLUGIN_EXTENSION} folder.`);
  }
  const dir = installedPluginsDir(configPath);
  await mkdir(dir, { recursive: true });
  // Staged in a folder the loader skips, then renamed into place, so a running companion never loads half a package.
  const staging = await mkdtemp(path.join(dir, ".install-"));
  try {
    let staged = path.join(staging, path.basename(source));
    if (source.endsWith(HOST_PLUGIN_ARCHIVE_EXTENSION)) staged = await extractHostPluginArchive(source, staging);
    else await cp(source, staged, { recursive: true });
    const plugin = await loadHostPluginPackage(staged);
    if (BUILT_IN_IDS.has(plugin.id)) throw new Error(`${plugin.id} is a built-in plugin and cannot be replaced.`);
    const target = path.join(dir, `${plugin.id}${HOST_PLUGIN_EXTENSION}`);
    await rm(target, { recursive: true, force: true });
    await rename(staged, target);
    console.log(
      `Installed ${plugin.id} (${[...plugin.commands.keys()].join(", ")}). A running Host Companion loads it now.`,
    );
  } finally {
    await rm(staging, { recursive: true, force: true });
  }
}

async function remove(id: string, configPath: string): Promise<void> {
  if (!isHostPluginId(id)) throw new Error(`${id} is not a plugin id.`);
  try {
    await rm(path.join(installedPluginsDir(configPath), `${id}${HOST_PLUGIN_EXTENSION}`), { recursive: true });
  } catch (error) {
    if (error instanceof Error && "code" in error && error.code === "ENOENT") {
      throw new Error(`No plugin ${id} is installed.`, { cause: error });
    }
    throw error;
  }
  console.log(`Removed ${id}.`);
}

async function notify(text: string, port: number | undefined, configPath: string): Promise<number> {
  const target = port ?? (await loadConfig(configPath)).port;
  const sent = await createHostClient(corePlugin, { url: `http://127.0.0.1:${target}/` }).notify({ text });
  if (sent.ok) return 0;
  console.error(
    `No Host Companion took the notice on port ${target} (${sent.message}). Is \`vitadeck-host-companion start\` running?`,
  );
  return 1;
}

async function main(): Promise<number> {
  const { positionals, values } = parseArgs({
    allowPositionals: true,
    options: {
      port: { type: "string" },
      plugin: { type: "string", multiple: true },
      config: { type: "string" },
      advertise: { type: "string" },
      help: { type: "boolean", short: "h" },
    },
  });
  const [command, ...rest] = positionals;
  const [arg] = rest;
  const configPath = values.config === undefined ? DEFAULT_CONFIG_PATH : path.resolve(userCwd, values.config);

  if (values.help) {
    console.log(USAGE);
    return 0;
  }
  if (command === "start" && rest.length === 0) {
    await start(values, configPath);
    return 0;
  }
  if (command === "link" && rest.length === 1 && arg !== undefined) {
    const outcome = await linkVita(arg, {
      configPath,
      port: parsePort(values.port),
      advertise: values.advertise,
    });
    (outcome.ok ? console.log : console.error)(outcome.message);
    return outcome.ok ? 0 : 1;
  }
  if (command === "install" && rest.length === 1 && arg !== undefined) {
    await install(arg, configPath);
    return 0;
  }
  if (command === "notify" && rest.length === 1 && arg !== undefined) {
    return notify(arg, parsePort(values.port), configPath);
  }
  if (command === "remove" && rest.length === 1 && arg !== undefined) {
    await remove(arg, configPath);
    return 0;
  }
  console.error(USAGE);
  return 1;
}

try {
  process.exitCode = await main();
} catch (error) {
  console.error(error instanceof Error ? error.message : String(error));
  process.exitCode = 1;
}
