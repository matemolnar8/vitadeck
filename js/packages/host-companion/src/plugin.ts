import { createHash } from "node:crypto";
import { mkdir, readdir, readFile, rename, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import { pathToFileURL } from "node:url";
import { isRecord, type ImplementedHostPlugin } from "@vitadeck/sdk/host-control";

export const HOST_PLUGIN_FILE_EXTENSION = ".vdplugin";

// Ids name the installed file, so they stay plain file names, as long as a Deck App Package name allows.
const PLUGIN_ID_PATTERN = /^[A-Za-z0-9][A-Za-z0-9_-]{0,126}$/u;

export function isHostPluginId(id: string): boolean {
  return PLUGIN_ID_PATTERN.test(id);
}

function isImplementedHostPlugin(value: unknown): value is ImplementedHostPlugin {
  return isRecord(value) && typeof value.id === "string" && value.commands instanceof Map;
}

/** Loads a Host Plugin File: one self-contained ES module that default-exports `implementHostPlugin(...)`. */
export async function loadHostPluginFile(file: string): Promise<ImplementedHostPlugin> {
  const source = await readFile(file);
  // Node imports no .vdplugin, and bundled CommonJS needs a file URL for createRequire(import.meta.url). A copy named
  // by its content also loads a rebuilt file fresh instead of from the module cache.
  const copy = path.join(
    os.tmpdir(),
    "vitadeck-host-companion",
    `${createHash("sha256").update(source).digest("hex")}.mjs`,
  );
  let module: unknown;
  try {
    await mkdir(path.dirname(copy), { recursive: true });
    // Renamed into place, so another companion loading the same file never imports a half-written copy.
    const temporary = `${copy}.${process.pid}.${Date.now()}.tmp`;
    await writeFile(temporary, source);
    await rename(temporary, copy);
    module = await import(pathToFileURL(copy).href);
  } catch (error) {
    throw new Error(`${file} failed to load: ${error instanceof Error ? error.message : String(error)}`, {
      cause: error,
    });
  }
  if (!isRecord(module) || !isImplementedHostPlugin(module.default)) {
    throw new Error(`${file} does not default-export implementHostPlugin(...).`);
  }
  if (!isHostPluginId(module.default.id)) {
    throw new Error(`${file} has plugin id "${module.default.id}"; ids must match ${PLUGIN_ID_PATTERN.toString()}.`);
  }
  return module.default;
}

/** The Host Plugin Files in `dir`, by file name. */
export async function hostPluginFilesIn(dir: string): Promise<string[]> {
  try {
    return (await readdir(dir))
      .filter((name) => name.endsWith(HOST_PLUGIN_FILE_EXTENSION))
      .toSorted()
      .map((name) => path.join(dir, name));
  } catch (error) {
    if (error instanceof Error && "code" in error && error.code === "ENOENT") return [];
    throw error;
  }
}
