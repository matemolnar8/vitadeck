import assert from "node:assert/strict";
import { readFile, readdir } from "node:fs/promises";
import { networkInterfaces } from "node:os";
import test from "node:test";
import { fileURLToPath } from "node:url";
import {
  command,
  corePlugin,
  createHostClient,
  defineHostPlugin,
  implementHostPlugin,
  isAbsent,
  systemStatsPlugin,
} from "@vitadeck/sdk/host-control";
import { loadHostPluginFile } from "../src/plugin";
import { MAX_REQUEST_BYTES, startHostCompanion, type HostCompanionOptions } from "../src/server";
import { systemStats } from "../src/system-stats";
import { greeterPlugin } from "./fixtures/greeter-plugin";
import { buildGreeterPluginFile, tempDir, writeTempFile } from "./fixtures/plugin-file";

type Fixture = { request: unknown; status: number; response: unknown };

const fixturesDir = fileURLToPath(new URL("../../../../tests/fixtures/host-control/", import.meta.url));

async function companion(t: test.TestContext, options: Partial<HostCompanionOptions> = {}) {
  return `http://127.0.0.1:${(await runningCompanion(t, options)).port}/`;
}

async function runningCompanion(t: test.TestContext, options: Partial<HostCompanionOptions> = {}) {
  const running = await startHostCompanion({
    port: 0,
    host: "127.0.0.1",
    name: "Fixture Host",
    platform: "darwin",
    log: () => {},
    ...options,
  });
  t.after(() => running.close());
  return running;
}

async function post(url: string, body: string, headers?: Record<string, string>) {
  const response = await fetch(`${url}command`, {
    method: "POST",
    headers: { "Content-Type": "application/json", ...headers },
    body,
  });
  return { status: response.status, headers: response.headers, body: await response.json() };
}

const commandFixtures = (await readdir(fixturesDir)).filter(
  (name) => name.endsWith(".json") && !name.startsWith("link-"),
);

for (const name of commandFixtures) {
  void test(`replays fixture ${name}`, async (t) => {
    const fx = JSON.parse(await readFile(`${fixturesDir}${name}`, "utf8")) as Fixture;
    const url = await companion(t);
    const response = await post(url, JSON.stringify(fx.request));
    assert.equal(response.status, fx.status);
    assert.deepEqual(response.body, fx.response);
    assert.equal(response.headers.get("content-type"), "application/json");
    assert.equal(response.headers.get("connection"), "close");
  });
}

void test("rejects a body over 64 KiB with payload_too_large", async (t) => {
  const url = await companion(t);
  const text = "x".repeat(MAX_REQUEST_BYTES);
  const response = await post(
    url,
    JSON.stringify({ protocolVersion: 1, plugin: "core", command: "echo", payload: { text } }),
  );
  assert.equal(response.status, 413);
  assert.deepEqual(response.body, {
    ok: false,
    code: "payload_too_large",
    message: "A command request may be at most 65536 bytes.",
  });
});

void test("rejects a wrong method, content type, route, and malformed JSON", async (t) => {
  const url = await companion(t);

  const get = await fetch(`${url}command`);
  assert.equal(get.status, 400);
  assert.deepEqual(await get.json(), {
    ok: false,
    code: "malformed_request",
    message: "Send commands with POST /command.",
  });

  const text = await post(url, "{}", { "Content-Type": "text/plain" });
  assert.equal(text.status, 400);
  assert.deepEqual(text.body, {
    ok: false,
    code: "malformed_request",
    message: "Send commands with Content-Type: application/json.",
  });

  const route = await fetch(`${url}other`, { method: "POST" });
  assert.equal(route.status, 404);
  assert.deepEqual(await route.json(), {
    ok: false,
    code: "unknown_command",
    message: "This Host Companion serves only POST /command.",
  });

  const malformed = await post(url, "{not json", { "Content-Type": "application/json; charset=utf-8" });
  assert.equal(malformed.status, 400);
  assert.deepEqual(malformed.body, {
    ok: false,
    code: "malformed_request",
    message: "The request body is not a Host Control command.",
  });
});

void test("serves the SDK client for core and system stats", async (t) => {
  const url = await companion(t, { plugins: [systemStats] });

  const core = createHostClient(corePlugin, { url });
  assert.deepEqual(await core.echo({ text: "hello" }), { ok: true, text: "hello" });
  const capabilities = await core.capabilities();
  assert.ok(capabilities.ok);
  assert.deepEqual(capabilities.plugins, [
    { id: "core", commands: ["capabilities", "echo"] },
    { id: "systemStats", commands: ["read"] },
  ]);

  const client = createHostClient(systemStatsPlugin, { url });
  // The first read samples the CPU twice; later reads measure since the previous one.
  for (const stats of [await client.read(), await client.read()]) {
    assert.ok(stats.ok);
    assert.ok(stats.cpuPercent >= 0 && stats.cpuPercent <= 100);
    assert.equal(stats.cpuPercent, Math.round(stats.cpuPercent * 10) / 10);
    assert.ok(stats.memoryUsedPercent > 0 && stats.memoryUsedPercent <= 100);
    assert.ok(Number.isInteger(stats.memoryTotalBytes) && stats.memoryTotalBytes > 0);
    assert.ok(Number.isInteger(stats.uptimeSeconds) && stats.uptimeSeconds > 0);
  }
});

void test("loads a bundled Host Plugin File and maps its throws, including its own HostCommandError", async (t) => {
  const plugin = await loadHostPluginFile(await buildGreeterPluginFile(t));
  const url = await companion(t, { plugins: [plugin] });
  const greeter = createHostClient(greeterPlugin, { url });

  assert.deepEqual(await greeter.greet({ name: "Máté" }), { ok: true, greeting: "Hello, Máté!" });
  assert.deepEqual(await greeter.crash(), { ok: false, code: "command_failed", message: "The greeter fell over." });
  assert.deepEqual(await greeter.launch(), {
    ok: false,
    code: "unsupported_platform",
    message: "Launching is not supported on this computer.",
  });
});

void test("rejects a file that is not a Host Plugin File", async (t) => {
  const dir = await tempDir(t);
  await assert.rejects(
    loadHostPluginFile(await writeTempFile(dir, "number.vdplugin", "export default 1;\n")),
    /does not default-export implementHostPlugin/u,
  );
  await assert.rejects(
    loadHostPluginFile(await writeTempFile(dir, "broken.vdplugin", "export default {\n")),
    /broken\.vdplugin failed to load/u,
  );
  await assert.rejects(
    loadHostPluginFile(
      await writeTempFile(dir, "escape.vdplugin", 'export default { id: "../x", commands: new Map() };\n'),
    ),
    /has plugin id "\.\.\/x"/u,
  );
  const digits = await loadHostPluginFile(
    await writeTempFile(dir, "digits.vdplugin", 'export default { id: "2048", commands: new Map() };\n'),
  );
  assert.equal(digits.id, "2048");
});

void test("setPlugins swaps the plugins, keeping core, and keeps them when ids clash", async (t) => {
  const running = await runningCompanion(t);
  const core = createHostClient(corePlugin, { url: `http://127.0.0.1:${running.port}/` });
  const ids = async () => {
    const capabilities = await core.capabilities();
    assert.ok(capabilities.ok);
    return capabilities.plugins.map((plugin) => plugin.id);
  };

  assert.deepEqual(await ids(), ["core"]);
  running.setPlugins([systemStats]);
  assert.deepEqual(await ids(), ["core", "systemStats"]);
  assert.throws(() => {
    running.setPlugins([systemStats, systemStats]);
  }, /share the id systemStats/u);
  assert.deepEqual(await ids(), ["core", "systemStats"]);
});

void test("refuses two plugins with the same id", async () => {
  const duplicate = implementHostPlugin(defineHostPlugin("core", { ping: command<undefined, object>(isAbsent) }), {
    ping: () => ({}),
  });
  await assert.rejects(
    startHostCompanion({ port: 0, host: "127.0.0.1", name: "x", plugins: [duplicate], log: () => {} }),
    /share the id core/u,
  );
});

void test("a busy port fails instead of moving", async (t) => {
  const url = await companion(t);
  const port = Number(new URL(url).port);
  await assert.rejects(startHostCompanion({ port, host: "127.0.0.1", name: "x", log: () => {} }), /already in use/u);
});

void test("once linked, answers only linked Vitas, loopback and own addresses", async (t) => {
  const lines: string[] = [];
  const url = await companion(t, {
    vitas: () => Promise.resolve(["192.168.1.177"]),
    remoteAddress: (request) => String(request.headers["x-peer"]),
    log: (line) => {
      lines.push(line);
    },
  });
  const echo = JSON.stringify({ protocolVersion: 1, plugin: "core", command: "echo", payload: { text: "secret" } });
  const as = (peer: string) => post(url, echo, { "X-Peer": peer });

  const own = Object.values(networkInterfaces()).flatMap((list) => list?.map((info) => info.address) ?? []);
  const allowed = ["::ffff:192.168.1.177", "192.168.1.177", "127.0.0.1", "::1", ...own];
  const statuses = await Promise.all(allowed.map(async (peer) => (await as(peer)).status));
  assert.deepEqual(new Set(statuses), new Set([200]));

  const forbidden = await as("::ffff:192.168.1.50");
  assert.equal(forbidden.status, 403);
  assert.deepEqual(forbidden.body, {
    ok: false,
    code: "forbidden",
    message: "This Host Companion only answers the VitaDeck it is linked with.",
  });

  assert.ok(lines.some((line) => / 192\.168\.1\.177 core\.echo ok \d+ms$/u.test(line)));
  assert.ok(lines.some((line) => / 192\.168\.1\.50 POST \/command forbidden \d+ms$/u.test(line)));
  assert.ok(lines.every((line) => !line.includes("secret")));
});

void test("before any link, answers every peer", async (t) => {
  const url = await companion(t, { remoteAddress: () => "10.0.0.9" });
  const response = await post(
    url,
    JSON.stringify({ protocolVersion: 1, plugin: "core", command: "echo", payload: { text: "a" } }),
  );
  assert.equal(response.status, 200);
});

void test("implementations are checked by tsc", () => {
  assert.throws(
    () =>
      // @ts-expect-error: `echo` is missing.
      implementHostPlugin(corePlugin, {
        capabilities: () => ({ protocolVersion: 1, name: "", platform: "", plugins: [] }),
      }),
    /no implementation for echo/u,
  );
  implementHostPlugin(corePlugin, {
    capabilities: () => ({ protocolVersion: 1, name: "", platform: "", plugins: [] }),
    // @ts-expect-error: echo must return `{ text: string }`.
    echo: ({ text }) => ({ text: text.length }),
  });
  implementHostPlugin(systemStatsPlugin, {
    // @ts-expect-error: `read` takes no payload.
    read: (_payload: { text: string }) => ({
      cpuPercent: 0,
      memoryUsedPercent: 0,
      memoryTotalBytes: 0,
      uptimeSeconds: 0,
    }),
  });
});
