import { createHash } from "node:crypto";
import { mkdir, readdir, readFile, writeFile } from "node:fs/promises";
import path from "node:path";
import { pathToFileURL } from "node:url";
import { HOST_PLUGIN_SCHEMA_VERSION, isRecord, type ImplementedHostPlugin } from "@vitadeck/sdk/host-control";
import { unzipSync } from "fflate";

export const HOST_PLUGIN_EXTENSION = ".vdplugin";
export const HOST_PLUGIN_ARCHIVE_EXTENSION = ".vdplugin.zip";

// Ids name the installed folder, so they stay plain file names, as long as a Deck App Package name allows.
const PLUGIN_ID_PATTERN = /^[A-Za-z0-9][A-Za-z0-9_-]{0,126}$/u;

export function isHostPluginId(id: string): boolean {
  return PLUGIN_ID_PATTERN.test(id);
}

function isImplementedHostPlugin(value: unknown): value is ImplementedHostPlugin {
  return isRecord(value) && typeof value.id === "string" && value.commands instanceof Map;
}

function message(error: unknown): string {
  return error instanceof Error ? error.message : String(error);
}

/**
 * Loads a Host Plugin Package: a `.vdplugin` folder whose `manifest.json` names an entry module that default-exports
 * `implementHostPlugin(...)`. The entry finds the package's other files through `import.meta.url`.
 */
export async function loadHostPluginPackage(dir: string): Promise<ImplementedHostPlugin> {
  const root = path.resolve(dir);
  const manifestPath = path.join(root, "manifest.json");
  let manifest: unknown;
  try {
    manifest = JSON.parse(await readFile(manifestPath, "utf8"));
  } catch (error) {
    throw new Error(`${dir} has no readable manifest.json: ${message(error)}`, { cause: error });
  }
  if (
    !isRecord(manifest) ||
    manifest.schemaVersion !== HOST_PLUGIN_SCHEMA_VERSION ||
    typeof manifest.entry !== "string"
  ) {
    throw new Error(
      `${manifestPath} is not a Host Plugin Package manifest (schemaVersion ${HOST_PLUGIN_SCHEMA_VERSION}).`,
    );
  }
  const entry = path.resolve(root, manifest.entry);
  if (!entry.startsWith(`${root}${path.sep}`)) throw new Error(`${manifestPath} names an entry outside the package.`);

  let module: unknown;
  try {
    // The query changes with the content, so a rebuilt entry loads fresh instead of from the module cache.
    const hash = createHash("sha256")
      .update(await readFile(entry))
      .digest("hex");
    module = await import(`${pathToFileURL(entry).href}?v=${hash}`);
  } catch (error) {
    throw new Error(`${dir} failed to load: ${message(error)}`, { cause: error });
  }
  if (!isRecord(module) || !isImplementedHostPlugin(module.default)) {
    throw new Error(`${dir} does not default-export implementHostPlugin(...).`);
  }
  if (!isHostPluginId(module.default.id)) {
    throw new Error(`${dir} has plugin id "${module.default.id}"; ids must match ${PLUGIN_ID_PATTERN.toString()}.`);
  }
  return module.default;
}

/** The Host Plugin Packages in `dir`, by folder name. */
export async function hostPluginPackagesIn(dir: string): Promise<string[]> {
  try {
    return (await readdir(dir, { withFileTypes: true }))
      .filter((entry) => entry.isDirectory() && entry.name.endsWith(HOST_PLUGIN_EXTENSION))
      .map((entry) => path.join(dir, entry.name))
      .toSorted();
  } catch (error) {
    if (error instanceof Error && "code" in error && error.code === "ENOENT") return [];
    throw error;
  }
}

/** Unpacks a `.vdplugin.zip`, which holds exactly one top-level `.vdplugin` folder, into `intoDir`. */
export async function extractHostPluginArchive(file: string, intoDir: string): Promise<string> {
  const entries = Object.entries(unzipSync(await readFile(file)));
  const roots = new Set(entries.map(([name]) => name.split("/")[0]));
  const [root] = roots;
  if (roots.size !== 1 || root === undefined || !root.endsWith(HOST_PLUGIN_EXTENSION)) {
    throw new Error(`${file} must hold exactly one top-level ${HOST_PLUGIN_EXTENSION} folder.`);
  }
  const files = entries.filter(([name]) => !name.endsWith("/"));
  const unsafe = files.find(
    ([name]) => name.includes("\\") || name.split("/").some((part) => part === "" || part === ".."),
  );
  if (unsafe) throw new Error(`${file} holds an unsafe path: ${unsafe[0]}`);
  await Promise.all(
    files.map(async ([name, data]) => {
      const target = path.join(intoDir, ...name.split("/"));
      await mkdir(path.dirname(target), { recursive: true });
      await writeFile(target, data);
    }),
  );
  return path.join(intoDir, root);
}
