import { mkdtemp, rm, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import type test from "node:test";
import { fileURLToPath } from "node:url";
import { rolldown } from "rolldown";

export async function tempDir(t: test.TestContext): Promise<string> {
  const dir = await mkdtemp(path.join(os.tmpdir(), "vitadeck-host-companion-"));
  t.after(() => rm(dir, { recursive: true, force: true }));
  return dir;
}

export async function writeTempFile(dir: string, name: string, contents: string): Promise<string> {
  const file = path.join(dir, name);
  await writeFile(file, contents);
  return file;
}

/** Bundles greeter-plugin.ts the way `vitadeck build` bundles a Deck App's `hostPlugin`. */
export async function buildGreeterPluginFile(t: test.TestContext, dir?: string): Promise<string> {
  const file = path.join(dir ?? (await tempDir(t)), "greeter.vdplugin");
  const bundle = await rolldown({
    input: fileURLToPath(new URL("greeter-plugin.ts", import.meta.url)),
    platform: "node",
    tsconfig: false,
  });
  await bundle.write({ file, format: "esm" });
  await bundle.close();
  return file;
}
