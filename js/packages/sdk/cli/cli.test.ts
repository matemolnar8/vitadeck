import assert from "node:assert/strict";
import { mkdtemp, readdir, readFile, rm, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import test from "node:test";
import { cliPath, execFileText, tsxBin } from "./test-project";

async function runCli(cwd: string, args: string[]): Promise<{ ok: boolean; output: string }> {
  try {
    const { stdout, stderr } = await execFileText(tsxBin, [cliPath, ...args], { cwd, timeout: 30_000 });
    return { ok: true, output: `${stdout}\n${stderr}` };
  } catch (error) {
    const failed = error as { stdout?: string; stderr?: string };
    return { ok: false, output: `${failed.stdout ?? ""}\n${failed.stderr ?? ""}` };
  }
}

async function makeTempDir(t: test.TestContext): Promise<string> {
  const dir = await mkdtemp(path.join(os.tmpdir(), "vitadeck-sdk-cli-"));
  t.after(async () => {
    await rm(dir, { recursive: true, force: true });
  });
  return dir;
}

void test("--help lists the commands", async (t) => {
  const result = await runCli(await makeTempDir(t), ["--help"]);
  assert.equal(result.ok, true, result.output);
  for (const command of ["build", "dev <runtime-upload-url>", "create <project-name>", "--force"]) {
    assert.ok(result.output.includes(command), result.output);
  }
});

void test("--version prints the SDK version", async (t) => {
  const sdkPackage = JSON.parse(await readFile(new URL("../package.json", import.meta.url), "utf8")) as {
    version: string;
  };
  const result = await runCli(await makeTempDir(t), ["--version"]);
  assert.equal(result.ok, true, result.output);
  assert.equal(result.output.trim(), sdkPackage.version);
});

void test("create refuses a non-empty directory unless --force is passed", async (t) => {
  const cwd = await makeTempDir(t);
  await writeFile(path.join(cwd, "keep.txt"), "mine\n");

  const refused = await runCli(cwd, ["create", "."]);
  assert.equal(refused.ok, false, refused.output);
  assert.match(refused.output, /is not empty/u);
  assert.deepEqual(await readdir(cwd), ["keep.txt"]);

  const forced = await runCli(cwd, ["create", ".", "--force"]);
  assert.equal(forced.ok, true, forced.output);
  assert.ok((await readdir(cwd)).includes("vitadeck.config.json"));
  assert.equal(await readFile(path.join(cwd, "keep.txt"), "utf8"), "mine\n");
});

void test("create scaffolds into a new directory", async (t) => {
  const cwd = await makeTempDir(t);
  const result = await runCli(cwd, ["create", "my-app"]);
  assert.equal(result.ok, true, result.output);
  assert.ok((await readdir(path.join(cwd, "my-app"))).includes("vitadeck.config.json"));
});
