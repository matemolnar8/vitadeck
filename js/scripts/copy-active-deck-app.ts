import { cp, mkdir, readFile, rm } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

type VitaDeckConfig = {
  deckAppPackage: string;
};

const scriptDir = path.dirname(fileURLToPath(import.meta.url));
const jsRoot = path.resolve(scriptDir, "..");
const repoRoot = path.resolve(jsRoot, "..");
const configPath = path.join(jsRoot, "vitadeck.config.json");
const outputDir = path.join(jsRoot, "dist", "deck-app");

async function readJson(filePath: string): Promise<unknown> {
  try {
    return JSON.parse(await readFile(filePath, "utf8"));
  } catch (error) {
    const message = error instanceof Error ? error.message : String(error);
    throw new Error(`Could not read ${path.relative(repoRoot, filePath)}: ${message}`, { cause: error });
  }
}

function assertObject(value: unknown, sourcePath: string): asserts value is Record<string, unknown> {
  if (typeof value !== "object" || value === null || Array.isArray(value)) {
    throw new Error(`${path.relative(repoRoot, sourcePath)} must contain a JSON object.`);
  }
}

function requireString(value: unknown, name: string, sourcePath: string): string {
  if (typeof value !== "string" || value.trim() === "") {
    throw new Error(`${path.relative(repoRoot, sourcePath)} must define a non-empty "${name}" string.`);
  }
  return value;
}

async function readConfig(): Promise<VitaDeckConfig> {
  const config = await readJson(configPath);
  assertObject(config, configPath);
  return {
    deckAppPackage: requireString(config.deckAppPackage, "deckAppPackage", configPath),
  };
}

const config = await readConfig();
const packagePath = path.resolve(jsRoot, config.deckAppPackage);
if (!packagePath.endsWith(".vdapp")) {
  throw new Error("deckAppPackage must point to a .vdapp directory.");
}

await rm(outputDir, { recursive: true, force: true });
await mkdir(path.dirname(outputDir), { recursive: true });
await cp(packagePath, outputDir, { recursive: true });

console.log(`Copied ${path.relative(repoRoot, packagePath)} to ${path.relative(repoRoot, outputDir)}`);
