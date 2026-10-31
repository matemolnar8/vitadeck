import { mkdir, mkdtemp, readdir, readFile, rm, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import type test from "node:test";
import { fileURLToPath } from "node:url";
import { zipSync } from "fflate";
import { rolldown } from "rolldown";

export async function tempDir(t: test.TestContext): Promise<string> {
  const dir = await mkdtemp(path.join(os.tmpdir(), "vitadeck-host-companion-"));
  t.after(() => rm(dir, { recursive: true, force: true }));
  return dir;
}

/** Writes `<dir>/<name>.vdplugin` the way `vitadeck build` does: `manifest.json` next to `companion.mjs`. */
export async function writePackage(dir: string, name: string, entrySource: string, manifest?: object): Promise<string> {
  const packageDir = path.join(dir, `${name}.vdplugin`);
  await mkdir(packageDir, { recursive: true });
  await writeFile(path.join(packageDir, "companion.mjs"), entrySource);
  await writeFile(
    path.join(packageDir, "manifest.json"),
    JSON.stringify(manifest ?? { schemaVersion: 1, name, version: "1.0.0", entry: "companion.mjs" }),
  );
  return packageDir;
}

/** Bundles greeter-plugin.ts into a package the way `vitadeck build` bundles a Deck App's `hostPlugin`. */
export async function buildGreeterPackage(t: test.TestContext, dir?: string): Promise<string> {
  const packageDir = await writePackage(dir ?? (await tempDir(t)), "greeter", "");
  const bundle = await rolldown({
    input: fileURLToPath(new URL("greeter-plugin.ts", import.meta.url)),
    platform: "node",
    tsconfig: false,
  });
  await bundle.write({ file: path.join(packageDir, "companion.mjs"), format: "esm" });
  await bundle.close();
  return packageDir;
}

/** Zips a package folder as its one top-level folder, like `vitadeck build` writes `<name>.vdplugin.zip`. */
export async function zipPackage(packageDir: string): Promise<string> {
  const root = path.basename(packageDir);
  const names = await readdir(packageDir);
  const contents = await Promise.all(names.map((name) => readFile(path.join(packageDir, name))));
  const files = Object.fromEntries(
    names.map((name, index) => [`${root}/${name}`, new Uint8Array(contents[index] ?? [])]),
  );
  await writeFile(`${packageDir}.zip`, zipSync(files));
  return `${packageDir}.zip`;
}
