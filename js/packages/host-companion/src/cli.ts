import { networkInterfaces } from "node:os";
import path from "node:path";
import { parseArgs } from "node:util";
import { DEFAULT_CONFIG_PATH, isPort, loadConfig, readConfig } from "./config";
import { linkVita } from "./link";
import { loadHostPlugin } from "./plugin";
import { startHostCompanion } from "./server";

const USAGE = `Usage: pnpm --dir js host-companion <command>

  start [--port N] [--plugin <specifier>]... [--config <path>]
      Serve Host Control commands on 0.0.0.0, port 8797 by default.
  link <vita-address>[:port] [--port N] [--advertise <ip>] [--config <path>]
      Link the VitaDeck showing that address (port 8798 by default) with the running companion.`;

// pnpm runs scripts from js/; INIT_CWD is where the user typed the command.
const userCwd = process.env.INIT_CWD ?? process.cwd();

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

async function start(values: { port?: string; plugin?: string[] }, configPath: string): Promise<void> {
  const config = await loadConfig(configPath);
  const specifiers = [...new Set([...config.plugins, ...(values.plugin ?? [])])];
  const plugins = await Promise.all(specifiers.map((specifier) => loadHostPlugin(specifier, userCwd)));
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
  console.log(`Plugins: ${["core", ...plugins.map((plugin) => plugin.id)].join(", ")}`);
  console.log(
    config.vitas.length > 0
      ? `Linked Vitas: ${config.vitas.join(", ")}`
      : "Open Host Control in the VitaDeck Shell, then run: pnpm --dir js host-companion link <vita-address>",
  );
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
  const configPath = values.config === undefined ? DEFAULT_CONFIG_PATH : path.resolve(userCwd, values.config);

  if (command === "start" && rest.length === 0) {
    await start(values, configPath);
    return 0;
  }
  if (command === "link" && rest.length === 1 && rest[0] !== undefined) {
    const outcome = await linkVita(rest[0], {
      configPath,
      port: parsePort(values.port),
      advertise: values.advertise,
    });
    (outcome.ok ? console.log : console.error)(outcome.message);
    return outcome.ok ? 0 : 1;
  }
  if (values.help) {
    console.log(USAGE);
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
