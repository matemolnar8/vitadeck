import assert from "node:assert/strict";
import { mkdir, mkdtemp, readdir, readFile, rm, symlink, writeFile } from "node:fs/promises";
import os from "node:os";
import path from "node:path";
import test from "node:test";
import { pathToFileURL } from "node:url";
import { unzipSync } from "fflate";
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

void test("the SDK and the Host Companion share one version, since one release tag publishes both", async () => {
  const [sdk, companion] = await Promise.all(
    ["../package.json", "../../host-companion/package.json"].map(
      async (manifest) => JSON.parse(await readFile(new URL(manifest, import.meta.url), "utf8")) as { version: string },
    ),
  );
  assert.equal(companion?.version, sdk?.version);
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
    hostPlugin: { entry: "./host/companion.ts" },
  });
  const pkg = JSON.parse(await readFile(path.join(projectRoot, "package.json"), "utf8")) as {
    scripts: Record<string, string>;
    devDependencies: Record<string, string>;
  };
  assert.equal(pkg.scripts.host, "vitadeck build && vitadeck-host-companion start --plugin ./dist");
  assert.equal(pkg.scripts.build, "vitadeck build");
  assert.ok("@vitadeck/host-companion" in pkg.devDependencies);
  assert.ok("typescript" in pkg.devDependencies);

  await linkSdk(projectRoot);
  await linkSdkBuildDependencies(projectRoot);
  await Promise.all(["react", "@types/react", "@types/node"].map((name) => linkDependency(projectRoot, name)));
  await execFileText(tscBin, ["-p", "."], { cwd: projectRoot });
  await execFileText(tscBin, ["-p", "host"], { cwd: projectRoot });

  const built = await runCli(projectRoot, ["build"]);
  assert.equal(built.ok, true, built.output);
  assert.match(built.output, /Wrote dist\/my-app\.vdplugin\.zip/u);
  const pluginDir = path.join(projectRoot, "dist", "my-app.vdplugin");
  assert.deepEqual(JSON.parse(await readFile(path.join(pluginDir, "manifest.json"), "utf8")), {
    schemaVersion: 1,
    name: "my-app",
    version: "0.1.0",
    entry: "companion.mjs",
  });
  assert.deepEqual(Object.keys(unzipSync(await readFile(`${pluginDir}.zip`))).toSorted(), [
    "my-app.vdplugin/companion.mjs",
    "my-app.vdplugin/manifest.json",
  ]);
  const source = await readFile(path.join(pluginDir, "companion.mjs"), "utf8");
  assert.doesNotMatch(source, /from "@vitadeck\/sdk/u, "the entry must be self-contained");
  const plugin = (
    (await import(pathToFileURL(path.join(pluginDir, "companion.mjs")).href)) as {
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

void test("build copies hostPlugin.files into the package and fails clearly on a missing entry", async (t) => {
  const cwd = await makeTempDir(t);
  assert.equal((await runCli(cwd, ["create", "my-app"])).ok, true);
  const projectRoot = path.join(cwd, "my-app");
  const configPath = path.join(projectRoot, "vitadeck.config.json");
  const config = JSON.parse(await readFile(configPath, "utf8")) as Record<string, unknown>;
  await linkSdkBuildDependencies(projectRoot);
  await linkDependency(projectRoot, "react");
  await mkdir(path.join(projectRoot, "host", "files", "scripts"), { recursive: true });
  await writeFile(
    path.join(projectRoot, "host", "companion.mjs"),
    'export default { id: "x", commands: new Map() };\n',
  );
  await writeFile(path.join(projectRoot, "host", "files", "scripts", "hello.sh"), "echo hello\n");

  await writeFile(
    configPath,
    JSON.stringify({ ...config, hostPlugin: { entry: "./host/companion.mjs", files: "./host/files" } }),
  );
  const built = await runCli(projectRoot, ["build", "--no-zip"]);
  assert.equal(built.ok, true, built.output);
  const copied = path.join(projectRoot, "dist", "my-app.vdplugin", "files", "scripts", "hello.sh");
  assert.equal(await readFile(copied, "utf8"), "echo hello\n");

  await writeFile(path.join(projectRoot, "host", "companion.mjs"), "export default {\n");
  const broken = await runCli(projectRoot, ["build", "--no-zip"]);
  assert.equal(broken.ok, false, broken.output);
  assert.match(
    await readFile(path.join(projectRoot, "dist", "my-app.vdplugin", "companion.mjs"), "utf8"),
    /id: "x"/u,
    "a broken build keeps the last good package",
  );

  await writeFile(configPath, JSON.stringify({ ...config, hostPlugin: { entry: "./host/companion.mjs", files: "." } }));
  const root = await runCli(projectRoot, ["build"]);
  assert.equal(root.ok, false, root.output);
  assert.match(root.output, /"hostPlugin\.files" must name a folder inside the project/u);

  await writeFile(configPath, JSON.stringify({ ...config, hostPlugin: { entry: "./host/missing.ts" } }));
  const missing = await runCli(projectRoot, ["build"]);
  assert.equal(missing.ok, false, missing.output);
  assert.match(missing.output, /hostPlugin\.entry file is missing/u);
});

void test("create --host-plugin-only scaffolds a standalone plugin that a Deck App can depend on", async (t) => {
  const cwd = await makeTempDir(t);
  const created = await runCli(cwd, ["create", "lights", "--host-plugin-only"]);
  assert.equal(created.ok, true, created.output);
  const pluginRoot = path.join(cwd, "lights");
  assert.deepEqual((await readdir(pluginRoot)).toSorted(), [
    ".gitignore",
    "host",
    "package.json",
    "vitadeck.config.json",
  ]);
  assert.deepEqual(JSON.parse(await readFile(path.join(pluginRoot, "vitadeck.config.json"), "utf8")), {
    name: "lights",
    outDir: "./dist",
    hostPlugin: { entry: "./host/companion.ts" },
  });
  const pkg = JSON.parse(await readFile(path.join(pluginRoot, "package.json"), "utf8")) as Record<string, unknown>;
  assert.deepEqual(pkg.exports, { ".": "./host/plugin.ts" });
  assert.equal(pkg.dependencies, undefined, "the SDK is a peer, so a Deck App and its plugins share one copy");
  assert.ok("@vitadeck/sdk" in (pkg.peerDependencies as object) && "@vitadeck/sdk" in (pkg.devDependencies as object));
  const both = await runCli(cwd, ["create", "both", "--host-plugin", "--host-plugin-only"]);
  assert.equal(both.ok, false);
  assert.match(both.output, /not both/u);

  await linkSdk(pluginRoot);
  await linkDependency(pluginRoot, "@types/node");
  await execFileText(tscBin, ["-p", "host"], { cwd: pluginRoot });
  const built = await runCli(pluginRoot, ["build"]);
  assert.equal(built.ok, true, built.output);
  assert.match(built.output, /Wrote dist\/lights\.vdplugin\.zip/u);
  assert.deepEqual((await readdir(path.join(pluginRoot, "dist"))).toSorted(), [
    "lights.vdplugin",
    "lights.vdplugin.zip",
  ]);

  // A separate Deck App imports the plugin's commands through the package's export, as from npm or a local path.
  assert.equal((await runCli(cwd, ["create", "deck"])).ok, true);
  const appRoot = path.join(cwd, "deck");
  await linkSdk(appRoot);
  await linkSdkBuildDependencies(appRoot);
  await Promise.all(["react", "@types/react"].map((name) => linkDependency(appRoot, name)));
  await symlink(pluginRoot, path.join(appRoot, "node_modules", "lights"), "dir");
  await writeFile(
    path.join(appRoot, "src", "App.tsx"),
    `import { createHostClient } from "@vitadeck/sdk/host-control";
import { hostPlugin } from "lights";

const lights = createHostClient(hostPlugin);

export default function App() {
  void lights.greet({ name: "Vita" });
  return null;
}
`,
  );
  await execFileText(tscBin, ["-p", "."], { cwd: appRoot });
  const app = await runCli(appRoot, ["build", "--no-zip"]);
  assert.equal(app.ok, true, app.output);
  assert.match(
    await readFile(path.join(appRoot, "dist", "deck.vdapp", "app.js"), "utf8"),
    /defineHostPlugin\("lights"/u,
  );
});

void test("a project needs a Deck App entry, a hostPlugin, or both, and dev takes a URL only for a Deck App", async (t) => {
  const cwd = await makeTempDir(t);
  await writeFile(path.join(cwd, "vitadeck.config.json"), JSON.stringify({ name: "x", outDir: "./dist" }));
  const neither = await runCli(cwd, ["build"]);
  assert.equal(neither.ok, false);
  assert.match(neither.output, /must define "entry" \(a Deck App\), "hostPlugin", or both/u);

  assert.equal((await runCli(cwd, ["create", "app"])).ok, true);
  const dev = await runCli(path.join(cwd, "app"), ["dev"]);
  assert.equal(dev.ok, false);
  assert.match(dev.output, /Usage: vitadeck dev <runtime-upload-url>/u);

  await writeFile(
    path.join(cwd, "vitadeck.config.json"),
    JSON.stringify({ name: "x", outDir: "./dist", hostPlugin: { entry: "./plugin.mjs" } }),
  );
  const pluginDev = await runCli(cwd, ["dev", "192.168.1.50"]);
  assert.equal(pluginDev.ok, false);
  assert.match(pluginDev.output, /no Deck App to upload/u);
});
