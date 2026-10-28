import { randomUUID } from "node:crypto";
import { mkdir, readFile, rename, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import { isRecord } from "@vitadeck/sdk/host-control";

export type HostCompanionConfig = {
  /** Generated once; VitaDeck keeps one link per id. */
  id: string;
  name: string;
  port: number;
  /** The IP to put in the link URL instead of the one routed to the Vita. */
  advertise: string | null;
  /** Linked Vita IPs. Once non-empty, only these, loopback and this computer's addresses get answers. */
  vitas: string[];
  /** Plugin specifiers: a path, or a package name whose `./companion` export is the implementation. */
  plugins: string[];
};

export const DEFAULT_CONFIG_PATH = path.join(os.homedir(), ".vitadeck", "host-companion.json");
export const DEFAULT_PORT = 8797;

function defaults(): HostCompanionConfig {
  return {
    id: randomUUID(),
    name: os.hostname(),
    port: DEFAULT_PORT,
    advertise: null,
    vitas: [],
    plugins: ["@vitadeck/host-plugin-system-stats"],
  };
}

export function isPort(value: unknown): value is number {
  return Number.isInteger(value) && typeof value === "number" && value > 0 && value <= 65_535;
}

const isNonEmptyString = (value: unknown): value is string => typeof value === "string" && value !== "";
const isStringArray = (value: unknown): value is string[] =>
  Array.isArray(value) && value.every((item) => typeof item === "string");

const validators: { [K in keyof HostCompanionConfig]: (value: unknown) => value is HostCompanionConfig[K] } = {
  id: isNonEmptyString,
  name: isNonEmptyString,
  port: isPort,
  advertise: (value): value is string | null => value === null || isNonEmptyString(value),
  vitas: isStringArray,
  plugins: isStringArray,
};

/** Reads the config file; missing fields take their defaults, wrongly typed ones throw. */
export async function readConfig(file: string): Promise<{ config: HostCompanionConfig; complete: boolean }> {
  let raw: unknown;
  try {
    raw = JSON.parse(await readFile(file, "utf8"));
  } catch (error) {
    if (error instanceof SyntaxError) throw new Error(`${file} is not valid JSON.`, { cause: error });
    throw error;
  }
  if (!isRecord(raw)) throw new Error(`${file} must hold a JSON object.`);

  const config = defaults();
  let complete = true;
  for (const key of Object.keys(validators) as (keyof HostCompanionConfig)[]) {
    if (!(key in raw)) {
      complete = false;
      continue;
    }
    const value = raw[key];
    if (!validators[key](value)) throw new Error(`${file}: "${key}" has the wrong type.`);
    Object.assign(config, { [key]: value });
  }
  return { config, complete };
}

export async function saveConfig(file: string, config: HostCompanionConfig): Promise<void> {
  await mkdir(path.dirname(file), { recursive: true });
  const temporary = `${file}.${process.pid}.tmp`;
  await writeFile(temporary, `${JSON.stringify(config, null, 2)}\n`);
  await rename(temporary, file);
}

/** Reads the config, creating it or filling in missing fields on first use. */
export async function loadConfig(file: string): Promise<HostCompanionConfig> {
  try {
    const { config, complete } = await readConfig(file);
    if (!complete) await saveConfig(file, config);
    return config;
  } catch (error) {
    if (!(error instanceof Error && "code" in error && error.code === "ENOENT")) throw error;
    const config = defaults();
    await saveConfig(file, config);
    return config;
  }
}
