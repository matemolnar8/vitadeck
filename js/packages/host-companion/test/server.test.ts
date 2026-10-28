import assert from "node:assert/strict";
import { readFile, readdir } from "node:fs/promises";
import { networkInterfaces } from "node:os";
import test from "node:test";
import { fileURLToPath } from "node:url";
import { systemStatsPlugin } from "@vitadeck/host-plugin-system-stats";
import { corePlugin, createHostClient, defineHostPlugin, command, isAbsent } from "@vitadeck/sdk/host-control";
import { implementHostPlugin } from "../src/index";
import { loadHostPlugin } from "../src/plugin";
import { MAX_REQUEST_BYTES, startHostCompanion, type HostCompanionOptions } from "../src/server";
import { greeterPlugin } from "./fixtures/greeter-plugin";

type Fixture = { request: unknown; status: number; response: unknown };

const fixturesDir = fileURLToPath(new URL("../../../../tests/fixtures/host-control/", import.meta.url));
const packageDir = fileURLToPath(new URL("..", import.meta.url));

async function companion(t: test.TestContext, options: Partial<HostCompanionOptions> = {}) {
  const running = await startHostCompanion({
    port: 0,
    host: "127.0.0.1",
    name: "Fixture Host",
    platform: "darwin",
    log: () => {},
    ...options,
  });
  t.after(() => running.close());
  return `http://127.0.0.1:${running.port}/`;
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
  const plugin = await loadHostPlugin("@vitadeck/host-plugin-system-stats", packageDir);
  const url = await companion(t, { plugins: [plugin] });

  const core = createHostClient(corePlugin, { url });
  assert.deepEqual(await core.echo({ text: "hello" }), { ok: true, text: "hello" });
  const capabilities = await core.capabilities();
  assert.ok(capabilities.ok);
  assert.deepEqual(capabilities.plugins, [
    { id: "core", commands: ["capabilities", "echo"] },
    { id: "systemStats", commands: ["read"] },
  ]);

  const systemStats = createHostClient(systemStatsPlugin, { url });
  // The first read samples the CPU twice; later reads measure since the previous one.
  for (const stats of [await systemStats.read(), await systemStats.read()]) {
    assert.ok(stats.ok);
    assert.ok(stats.cpuPercent >= 0 && stats.cpuPercent <= 100);
    assert.equal(stats.cpuPercent, Math.round(stats.cpuPercent * 10) / 10);
    assert.ok(stats.memoryUsedPercent > 0 && stats.memoryUsedPercent <= 100);
    assert.ok(Number.isInteger(stats.memoryTotalBytes) && stats.memoryTotalBytes > 0);
    assert.ok(Number.isInteger(stats.uptimeSeconds) && stats.uptimeSeconds > 0);
  }
});

void test("loads a plugin from a file path and maps implementation throws", async (t) => {
  const plugin = await loadHostPlugin("./test/fixtures/greeter-plugin.ts", packageDir);
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

void test("rejects a module that does not default-export an implementation", async () => {
  await assert.rejects(loadHostPlugin("./src/core.ts", packageDir), /does not default-export implementHostPlugin/u);
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
