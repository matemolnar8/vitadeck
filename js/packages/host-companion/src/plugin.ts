import { readdir, readFile } from "node:fs/promises";
import path from "node:path";
import { isRecord, type ImplementedHostPlugin } from "@vitadeck/sdk/host-control";

export const HOST_PLUGIN_FILE_EXTENSION = ".vdplugin";

const PLUGIN_ID_PATTERN = /^[A-Za-z][A-Za-z0-9_-]{0,62}$/u;

export function isHostPluginId(id: string): boolean {
  return PLUGIN_ID_PATTERN.test(id);
}

function isImplementedHostPlugin(value: unknown): value is ImplementedHostPlugin {
  return isRecord(value) && typeof value.id === "string" && value.commands instanceof Map;
}

/** Loads a Host Plugin File: one self-contained ES module that default-exports `implementHostPlugin(...)`. */
export async function loadHostPluginFile(file: string): Promise<ImplementedHostPlugin> {
  const source = await readFile(file);
  let module: unknown;
  try {
    // Node imports no unknown extension from a file URL. A data: URL also changes with the content, so a rebuilt
    // file loads fresh instead of from the module cache.
    module = await import(`data:text/javascript;base64,${source.toString("base64")}`);
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

/** The Host Plugin Files `install` copied into `dir`, by file name. */
export async function installedHostPluginFiles(dir: string): Promise<string[]> {
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
