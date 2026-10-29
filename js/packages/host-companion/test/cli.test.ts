import assert from "node:assert/strict";
import { execFile, spawn } from "node:child_process";
import { access, mkdir, readdir } from "node:fs/promises";
import { createServer } from "node:net";
import path from "node:path";
import test from "node:test";
import { fileURLToPath } from "node:url";
import { createHostClient } from "@vitadeck/sdk/host-control";
import { greeterPlugin } from "./fixtures/greeter-plugin";
import { buildGreeterPluginFile, tempDir, writeTempFile } from "./fixtures/plugin-file";

const cliPath = fileURLToPath(new URL("../src/cli.ts", import.meta.url));
const tsxBin = fileURLToPath(new URL("../../../node_modules/.bin/tsx", import.meta.url));

function runCli(args: string[], cwd: string): Promise<{ code: number; output: string }> {
  return new Promise((resolve) => {
    execFile(tsxBin, [cliPath, ...args], { cwd, env: { ...process.env, INIT_CWD: cwd } }, (error, stdout, stderr) => {
      resolve({ code: error ? Number(error.code ?? 1) : 0, output: `${stdout}${stderr}` });
    });
  });
}

function freePort(): Promise<number> {
  return new Promise((resolve, reject) => {
    const server = createServer();
    server.once("error", reject);
    server.listen(0, "127.0.0.1", () => {
      const { port } = server.address() as { port: number };
      server.close(() => {
        resolve(port);
      });
    });
  });
}

/** Starts `start` and returns a function that waits for a line of its output matching `pattern`. */
function startCompanion(t: test.TestContext, args: string[], cwd: string) {
  const child = spawn(tsxBin, [cliPath, "start", ...args], { cwd, env: { ...process.env, INIT_CWD: cwd } });
  t.after(() => {
    child.kill();
  });
  let output = "";
  let waiter: (() => void) | undefined;
  const onData = (chunk: Buffer) => {
    output += chunk.toString();
    waiter?.();
  };
  child.stdout.on("data", onData);
  child.stderr.on("data", onData);
  return (pattern: RegExp) =>
    new Promise<void>((resolve, reject) => {
      const timer = setTimeout(() => {
        reject(new Error(`No line matched ${pattern.toString()} in:\n${output}`));
      }, 10_000);
      waiter = () => {
        if (!output.split("\n").some((line) => pattern.test(line))) return;
        output = "";
        waiter = undefined;
        clearTimeout(timer);
        resolve();
      };
      waiter();
    });
}

function exists(file: string): Promise<boolean> {
  return access(file).then(
    () => true,
    () => false,
  );
}

void test("install loads a Host Plugin File into a running companion, remove unloads it", async (t) => {
  const dir = await tempDir(t);
  const config = path.join(dir, "home", "host-companion.json");
  await buildGreeterPluginFile(t, dir);
  const port = await freePort();
  const waitFor = startCompanion(t, ["--port", String(port), "--config", config], dir);
  await waitFor(/^Plugins: core, systemStats$/u);
  const greeter = createHostClient(greeterPlugin, { url: `http://127.0.0.1:${port}/` });

  const installed = await runCli(["install", "greeter.vdplugin", "--config", config], dir);
  assert.equal(installed.code, 0, installed.output);
  assert.match(installed.output, /Installed greeter \(greet, crash, launch\)/u);
  assert.deepEqual(await readdir(path.join(dir, "home", "plugins")), ["greeter.vdplugin"]);
  await waitFor(/^Reloaded plugins: core, systemStats, greeter$/u);
  assert.deepEqual(await greeter.greet({ name: "Máté" }), { ok: true, greeting: "Hello, Máté!" });

  const removed = await runCli(["remove", "greeter", "--config", config], dir);
  assert.equal(removed.code, 0, removed.output);
  assert.equal(await exists(path.join(dir, "home", "plugins", "greeter.vdplugin")), false);
  await waitFor(/^Reloaded plugins: core, systemStats$/u);
  const gone = await greeter.greet({ name: "Máté" });
  assert.equal(gone.ok ? "ok" : gone.code, "unknown_plugin");
});

void test("a --plugin file reloads when it is rebuilt, and a broken rebuild keeps the previous one", async (t) => {
  const dir = await tempDir(t);
  const config = path.join(dir, "host-companion.json");
  await mkdir(path.join(dir, "dist"));
  await buildGreeterPluginFile(t, path.join(dir, "dist"));
  const port = await freePort();
  const waitFor = startCompanion(
    t,
    ["--port", String(port), "--config", config, "--plugin", "dist/greeter.vdplugin"],
    dir,
  );
  await waitFor(/^Plugins: core, systemStats, greeter$/u);
  const greeter = createHostClient(greeterPlugin, { url: `http://127.0.0.1:${port}/` });
  assert.deepEqual(await greeter.greet({ name: "Vita" }), { ok: true, greeting: "Hello, Vita!" });

  await writeTempFile(
    path.join(dir, "dist"),
    "greeter.vdplugin",
    `export default {
  id: "greeter",
  commands: new Map([["greet", { isPayload: () => true, run: ({ name }) => ({ greeting: "Hi again, " + name }) }]]),
};
`,
  );
  await waitFor(/^Reloaded plugins: core, systemStats, greeter$/u);
  assert.deepEqual(await greeter.greet({ name: "Vita" }), { ok: true, greeting: "Hi again, Vita" });

  await writeTempFile(path.join(dir, "dist"), "greeter.vdplugin", "export default {\n");
  await waitFor(/^Kept the previous plugins: .*greeter\.vdplugin failed to load/u);
  assert.deepEqual(await greeter.greet({ name: "Vita" }), { ok: true, greeting: "Hi again, Vita" });
});

void test("install refuses a built-in id and a file that is not a .vdplugin", async (t) => {
  const dir = await tempDir(t);
  const config = path.join(dir, "host-companion.json");
  await writeTempFile(dir, "stats.vdplugin", 'export default { id: "systemStats", commands: new Map() };\n');
  await writeTempFile(dir, "plugin.js", 'export default { id: "x", commands: new Map() };\n');

  const builtIn = await runCli(["install", "stats.vdplugin", "--config", config], dir);
  assert.equal(builtIn.code, 1);
  assert.match(builtIn.output, /systemStats is a built-in plugin/u);
  const wrongExtension = await runCli(["install", "plugin.js", "--config", config], dir);
  assert.equal(wrongExtension.code, 1);
  assert.match(wrongExtension.output, /is not a \.vdplugin file/u);
  const unknown = await runCli(["remove", "nothing", "--config", config], dir);
  assert.equal(unknown.code, 1);
  assert.match(unknown.output, /No plugin nothing is installed/u);
});
