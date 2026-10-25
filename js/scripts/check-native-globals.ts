import { readdir, readFile } from "node:fs/promises";
import path from "node:path";
import { fileURLToPath } from "node:url";

const jsRoot = fileURLToPath(new URL("..", import.meta.url));
const jslibDir = path.join(jsRoot, "../src/jslib");
const declarationFiles = [
  path.join(jsRoot, "packages/runtime/src/runtime-native.d.ts"),
  path.join(jsRoot, "packages/sdk/src/deck-app-globals.d.ts"),
];
// Registered natively but only reachable through `console`, which log.c builds from them.
const undeclaredNativeGlobals = new Set(["logInfo", "logDebug", "logWarn", "logError"]);

async function collectNames(files: string[], pattern: RegExp): Promise<Set<string>> {
  const sources = await Promise.all(files.map((file) => readFile(file, "utf8")));
  return new Set(sources.flatMap((source) => Array.from(source.matchAll(pattern), (match) => match[1] ?? "")));
}

const cFiles = (await readdir(jslibDir)).filter((name) => name.endsWith(".c")).map((name) => path.join(jslibDir, name));
const registered = await collectNames(cFiles, /js_set_global_function\(\s*ctx,\s*"(\w+)"/gu);
const declared = await collectNames(declarationFiles, /^\s*function (\w+)\(/gmu);

if (registered.size === 0) {
  console.error(`No js_set_global_function calls found in ${jslibDir}.`);
  process.exit(1);
}

const undeclared = [...registered].filter((name) => !declared.has(name) && !undeclaredNativeGlobals.has(name));
const unregistered = [...declared].filter((name) => name.startsWith("native") && !registered.has(name));
for (const name of undeclared) console.error(`${name} is registered in src/jslib but not declared.`);
for (const name of unregistered) console.error(`${name} is declared but not registered in src/jslib.`);
if (undeclared.length > 0 || unregistered.length > 0) process.exit(1);

console.log(`Native globals match: ${registered.size} registered in src/jslib.`);
