import assert from "node:assert/strict";
import { mkdtemp, readFile, rm, writeFile } from "node:fs/promises";
import { createServer, type Server } from "node:http";
import type { AddressInfo } from "node:net";
import os from "node:os";
import path from "node:path";
import test from "node:test";
import { fileURLToPath } from "node:url";
import { loadConfig, saveConfig } from "../src/config";
import { linkVita, parseVitaAddress } from "../src/link";
import { startHostCompanion } from "../src/server";

type Fixture = { status: number; response: unknown };

const fixturesDir = fileURLToPath(new URL("../../../../tests/fixtures/host-control/", import.meta.url));

async function fixture(name: string): Promise<Fixture> {
  return JSON.parse(await readFile(`${fixturesDir}${name}.json`, "utf8")) as Fixture;
}

async function listen(server: Server): Promise<number> {
  await new Promise<void>((resolve) => {
    server.listen(0, "127.0.0.1", resolve);
  });
  return (server.address() as AddressInfo).port;
}

function close(server: Server): Promise<void> {
  return new Promise((resolve) => {
    server.close(() => {
      resolve();
    });
  });
}

/** A port nothing listens on. */
async function freePort(): Promise<number> {
  const server = createServer();
  const port = await listen(server);
  await close(server);
  return port;
}

/** A VitaDeck link endpoint that records requests and answers with a fixture. */
async function stubVita(t: test.TestContext, fx: Fixture) {
  const received: { method?: string; url?: string; body: Record<string, unknown> }[] = [];
  const server = createServer((request, response) => {
    let body = "";
    request.on("data", (chunk: Buffer) => {
      body += chunk.toString("utf8");
    });
    request.on("end", () => {
      received.push({ method: request.method, url: request.url, body: JSON.parse(body) as Record<string, unknown> });
      response.writeHead(fx.status, { "Content-Type": "application/json", Connection: "close" });
      response.end(JSON.stringify(fx.response));
    });
  });
  const port = await listen(server);
  t.after(() => close(server));
  return { port, received };
}

async function setup(t: test.TestContext, withCompanion = true) {
  const dir = await mkdtemp(path.join(os.tmpdir(), "host-companion-"));
  t.after(() => rm(dir, { recursive: true, force: true }));
  const configPath = path.join(dir, "config.json");
  const config = await loadConfig(configPath);
  if (!withCompanion) return { configPath, config, port: 1 };
  const running = await startHostCompanion({ port: 0, host: "127.0.0.1", name: config.name, log: () => {} });
  t.after(() => running.close());
  return { configPath, config, port: running.port };
}

void test("parses a Vita address with an optional port", () => {
  assert.deepEqual(parseVitaAddress("192.168.1.177"), { host: "192.168.1.177", port: 8787 });
  assert.deepEqual(parseVitaAddress("192.168.1.177:9000"), { host: "192.168.1.177", port: 9000 });
  assert.equal(parseVitaAddress("192.168.1.177/link"), undefined);
});

void test("first use writes a config with a generated id", async (t) => {
  const { configPath, config } = await setup(t, false);
  assert.match(config.id, /^[0-9a-f-]{36}$/u);
  assert.deepEqual(JSON.parse(await readFile(configPath, "utf8")), config);
  assert.deepEqual(await loadConfig(configPath), config);
});

void test("links, then remembers the Vita", async (t) => {
  const { configPath, config, port } = await setup(t);
  const vita = await stubVita(t, await fixture("link-ok"));

  const outcome = await linkVita(`127.0.0.1:${vita.port}`, { configPath, port, advertise: "192.168.1.20" });

  assert.ok(outcome.ok, outcome.message);
  assert.deepEqual(vita.received, [
    {
      method: "POST",
      url: "/link",
      body: { protocolVersion: 1, id: config.id, name: config.name, url: `http://192.168.1.20:${port}/` },
    },
  ]);
  assert.deepEqual((await loadConfig(configPath)).vitas, ["127.0.0.1"]);

  await linkVita(`127.0.0.1:${vita.port}`, { configPath, port });
  assert.deepEqual((await loadConfig(configPath)).vitas, ["127.0.0.1"]);
});

void test("advertises the address routed to the Vita by default", async (t) => {
  const { configPath, port } = await setup(t);
  const vita = await stubVita(t, await fixture("link-ok"));
  await linkVita(`127.0.0.1:${vita.port}`, { configPath, port });
  assert.equal(vita.received[0]?.body.url, `http://127.0.0.1:${port}/`);
});

void test("reports a protocol version mismatch and saves nothing", async (t) => {
  const { configPath, port } = await setup(t);
  const vita = await stubVita(t, await fixture("link-version-mismatch"));

  const outcome = await linkVita(`127.0.0.1:${vita.port}`, { configPath, port });

  assert.deepEqual(outcome, {
    ok: false,
    message:
      "VitaDeck refused the link (unsupported_protocol_version): VitaDeck speaks Host Control protocol version 1.",
    retry: false,
  });
  assert.deepEqual((await loadConfig(configPath)).vitas, []);
});

void test("a relink asks VitaDeck to update its link only, and keeps the Vita", async (t) => {
  const { configPath, config, port } = await setup(t);
  await saveConfig(configPath, { ...config, vitas: ["127.0.0.1"] });
  const vita = await stubVita(t, await fixture("link-ok"));

  const outcome = await linkVita(`127.0.0.1:${vita.port}`, { configPath, port, relink: true });

  assert.ok(outcome.ok, outcome.message);
  assert.match(outcome.message, /^Relinked /u);
  assert.equal(vita.received[0]?.body.relink, true);
  assert.deepEqual((await loadConfig(configPath)).vitas, ["127.0.0.1"]);
});

void test("a Vita that forgot this computer is dropped on relink", async (t) => {
  const { configPath, config, port } = await setup(t);
  await saveConfig(configPath, { ...config, vitas: ["127.0.0.1", "192.168.1.50"] });
  const vita = await stubVita(t, await fixture("link-relink-unknown"));

  const outcome = await linkVita(`127.0.0.1:${vita.port}`, { configPath, port, relink: true });

  assert.ok(!outcome.ok && !outcome.retry);
  assert.match(outcome.message, /forgot this computer/u);
  assert.deepEqual((await loadConfig(configPath)).vitas, ["192.168.1.50"]);
});

void test("reports an unreachable Vita", async (t) => {
  const { configPath, port } = await setup(t);
  const outcome = await linkVita(`127.0.0.1:${await freePort()}`, { configPath, port });

  assert.ok(!outcome.ok && outcome.retry);
  assert.match(
    outcome.message,
    /^Could not reach VitaDeck at http:\/\/127\.0\.0\.1:\d+\/link \(.*\)\. Check that Upload and link is on/u,
  );
});

void test("refuses to link before start is running", async (t) => {
  const { configPath } = await setup(t, false);
  const outcome = await linkVita("127.0.0.1", { configPath, port: await freePort() });

  assert.equal(outcome.ok, false);
  assert.match(
    outcome.message,
    /^No Host Companion answers on port \d+ .*Run `vitadeck-host-companion start` first\.$/u,
  );
});

void test("rejects a wrongly typed config field", async (t) => {
  const { configPath } = await setup(t, false);
  await writeFile(configPath, JSON.stringify({ port: "8797" }));
  await assert.rejects(loadConfig(configPath), /"port" has the wrong type/u);
});
