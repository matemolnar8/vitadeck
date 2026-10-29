import assert from "node:assert/strict";
import { mkdtemp, readdir, readFile, rm, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import test from "node:test";
import {
  cliPath,
  execFileText,
  linkDependency,
  linkSdk,
  linkSdkBuildDependencies,
  tscBin,
  tsxBin,
} from "./test-project";

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

void test("create --host-plugin scaffolds a Deck App and Host Plugin that typecheck, build and run", async (t) => {
  const cwd = await makeTempDir(t);
  const created = await runCli(cwd, ["create", "my-app", "--host-plugin"]);
  assert.equal(created.ok, true, created.output);
  const projectRoot = path.join(cwd, "my-app");

  const config = JSON.parse(await readFile(path.join(projectRoot, "vitadeck.config.json"), "utf8")) as unknown;
  assert.deepEqual(config, {
    name: "my-app",
    entry: "./src/App.tsx",
    outDir: "./dist",
    hostPlugin: "./host/companion.ts",
  });
  const pkg = JSON.parse(await readFile(path.join(projectRoot, "package.json"), "utf8")) as {
    scripts: Record<string, string>;
    devDependencies: Record<string, string>;
  };
  assert.equal(pkg.scripts.host, "vitadeck build && vitadeck-host-companion start --plugin ./dist/my-app.vdplugin");
  assert.equal(pkg.scripts.build, "vitadeck build");
  assert.ok("@vitadeck/host-companion" in pkg.devDependencies);
  assert.ok("typescript" in pkg.devDependencies);

  await linkSdk(projectRoot);
  await linkSdkBuildDependencies(projectRoot);
  await Promise.all(["react", "@types/react", "@types/node"].map((name) => linkDependency(projectRoot, name)));
  await execFileText(tscBin, ["-p", "."], { cwd: projectRoot });
  await execFileText(tscBin, ["-p", "host"], { cwd: projectRoot });

  const built = await runCli(projectRoot, ["build", "--no-zip"]);
  assert.equal(built.ok, true, built.output);
  assert.match(built.output, /Wrote dist\/my-app\.vdplugin/u);
  const source = await readFile(path.join(projectRoot, "dist", "my-app.vdplugin"));
  assert.doesNotMatch(source.toString("utf8"), /from "@vitadeck\/sdk/u, "the plugin file must be self-contained");
  const plugin = (
    (await import(`data:text/javascript;base64,${source.toString("base64")}`)) as {
      default: {
        id: string;
        commands: Map<string, { isPayload: (value: unknown) => boolean; run: (payload: unknown) => unknown }>;
      };
    }
  ).default;
  assert.equal(plugin.id, "my-app");
  const greet = plugin.commands.get("greet");
  assert.ok(greet);
  assert.equal(greet.isPayload({ name: "Vita" }), true);
  assert.equal(greet.isPayload({}), false);
  assert.deepEqual(await greet.run({ name: "Vita" }), { message: `Hello Vita, this is ${os.hostname()}.` });
});

void test("build fails clearly when hostPlugin points at a missing file", async (t) => {
  const cwd = await makeTempDir(t);
  assert.equal((await runCli(cwd, ["create", "my-app"])).ok, true);
  const projectRoot = path.join(cwd, "my-app");
  const configPath = path.join(projectRoot, "vitadeck.config.json");
  const config = JSON.parse(await readFile(configPath, "utf8")) as Record<string, unknown>;
  await writeFile(configPath, JSON.stringify({ ...config, hostPlugin: "./host/missing.ts" }));
  await linkSdkBuildDependencies(projectRoot);
  await linkDependency(projectRoot, "react");

  const built = await runCli(projectRoot, ["build"]);
  assert.equal(built.ok, false, built.output);
  assert.match(built.output, /hostPlugin file is missing/u);
});
