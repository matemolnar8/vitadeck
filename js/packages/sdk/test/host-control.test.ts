import assert from "node:assert/strict";
import { readFile } from "node:fs/promises";
import test from "node:test";
import { fileURLToPath } from "node:url";
import {
  HOST_CONTROL_WIRE_ERRORS,
  corePlugin,
  createHostClient,
  linkedHosts,
  type HostTransport,
} from "../src/host-control/index";

type Fixture = { request: Record<string, unknown>; status: number; response: Record<string, unknown> };

const fixturesDir = fileURLToPath(new URL("../../../../tests/fixtures/host-control/", import.meta.url));

async function fixture(name: string): Promise<Fixture> {
  return JSON.parse(await readFile(`${fixturesDir}${name}.json`, "utf8")) as Fixture;
}

type Sent = { url: string; headers: Record<string, string>; body: unknown };

function replay(fx: Fixture, sent: Sent[] = []): HostTransport {
  return (url, init) => {
    sent.push({ url, headers: init.headers, body: JSON.parse(init.body) });
    return Promise.resolve({ status: fx.status, text: () => Promise.resolve(JSON.stringify(fx.response)) });
  };
}

function answering(status: number, body: string): HostTransport {
  return () => Promise.resolve({ status, text: () => Promise.resolve(body) });
}

const COMPANION_URL = "http://192.168.1.20:8797/";

const nativeGlobal = globalThis as { nativeGetHostControlLinks?: () => string };

function withLinks(json: string | undefined, t: test.TestContext) {
  const previous = nativeGlobal.nativeGetHostControlLinks;
  if (json === undefined) delete nativeGlobal.nativeGetHostControlLinks;
  else nativeGlobal.nativeGetHostControlLinks = () => json;
  t.after(() => {
    if (previous) nativeGlobal.nativeGetHostControlLinks = previous;
    else delete nativeGlobal.nativeGetHostControlLinks;
  });
}

void test("echo sends the fixture envelope to {url}command and returns its fields", async () => {
  const fx = await fixture("echo");
  const sent: Sent[] = [];
  const client = createHostClient(corePlugin, { url: COMPANION_URL, transport: replay(fx, sent) });
  const reply = await client.echo({ text: "hi" });
  assert.deepEqual(reply, fx.response);
  assert.equal(sent[0]?.url, `${COMPANION_URL}command`);
  assert.equal(sent[0]?.headers["Content-Type"], "application/json");
  assert.deepEqual(sent[0]?.body, fx.request);
});

void test("capabilities omits the payload", async () => {
  const fx = await fixture("capabilities");
  const sent: Sent[] = [];
  const reply = await createHostClient(corePlugin, {
    url: "http://192.168.1.20:8797",
    transport: replay(fx, sent),
  }).capabilities();
  assert.deepEqual(sent[0]?.body, fx.request);
  assert.equal(sent[0]?.url, `${COMPANION_URL}command`);
  assert.ok(reply.ok);
  assert.deepEqual(reply.plugins, [{ id: "core", commands: ["capabilities", "echo"] }]);
});

for (const name of ["command-version-mismatch", "unknown-plugin", "unknown-command", "invalid-payload"]) {
  void test(`${name} fixture maps to its code and status`, async () => {
    const fx = await fixture(name);
    const code = fx.response.code as keyof typeof HOST_CONTROL_WIRE_ERRORS;
    assert.equal(HOST_CONTROL_WIRE_ERRORS[code], fx.status);
    const reply = await createHostClient(corePlugin, { url: COMPANION_URL, transport: replay(fx) }).echo({
      text: "hi",
    });
    assert.deepEqual(reply, fx.response);
  });
}

void test("an unknown code becomes command_failed and keeps the message", async () => {
  const reply = await createHostClient(corePlugin, {
    url: COMPANION_URL,
    transport: answering(500, JSON.stringify({ ok: false, code: "fan_on_fire", message: "Too hot." })),
  }).echo({ text: "hi" });
  assert.deepEqual(reply, { ok: false, code: "command_failed", message: "Too hot." });
});

void test("a non-JSON or ok-less answer is bad_response", async () => {
  const bodies = ["<html>", JSON.stringify({ text: "hi" }), "[]"];
  const replies = await Promise.all(
    bodies.map((body) =>
      createHostClient(corePlugin, { url: COMPANION_URL, transport: answering(200, body) }).echo({ text: "hi" }),
    ),
  );
  assert.deepEqual(
    replies.map((reply) => (reply.ok ? "ok" : reply.code)),
    bodies.map(() => "bad_response"),
  );
});

void test("a rejected transport is unreachable", async () => {
  const reply = await createHostClient(corePlugin, {
    url: COMPANION_URL,
    transport: () => Promise.reject(new Error("Connection refused")),
  }).echo({ text: "hi" });
  assert.equal(reply.ok ? "ok" : reply.code, "unreachable");
  assert.match(reply.ok ? "" : reply.message, /Connection refused/u);
});

void test("a slow companion times out without waiting for it", async () => {
  const reply = await createHostClient(corePlugin, {
    url: COMPANION_URL,
    timeoutMs: 20,
    transport: () => new Promise(() => {}),
  }).echo({ text: "hi" });
  assert.equal(reply.ok ? "ok" : reply.code, "timeout");
});

for (const links of [undefined, "", "[]", "not json"]) {
  void test(`with links ${String(links)}, calls fail not_linked without the network`, async (t) => {
    let calls = 0;
    const transport: HostTransport = () => {
      calls++;
      return Promise.reject(new Error("unused"));
    };
    withLinks(links, t);
    const reply = await createHostClient(corePlugin, { transport }).echo({ text: "hi" });
    assert.equal(reply.ok ? "ok" : reply.code, "not_linked");
    assert.equal(calls, 0);
  });
}

void test("calls go to the most recent link, or to hostId, read on every call", async (t) => {
  const links = [
    { id: "b", name: "Windows PC", url: "http://192.168.1.30:8797/" },
    { id: "a", name: "Studio Mac", url: "http://192.168.1.20:8797/" },
  ];
  withLinks(JSON.stringify([...links, { id: 3 }]), t);
  assert.deepEqual(linkedHosts(), links);

  const fx = await fixture("echo");
  const sent: Sent[] = [];
  await createHostClient(corePlugin, { transport: replay(fx, sent) }).echo({ text: "hi" });
  const mac = createHostClient(corePlugin, { hostId: "a", transport: replay(fx, sent) });
  await mac.echo({ text: "hi" });
  assert.deepEqual(
    sent.map((s) => s.url),
    ["http://192.168.1.30:8797/command", "http://192.168.1.20:8797/command"],
  );

  withLinks(JSON.stringify([links[0]]), t);
  const reply = await mac.echo({ text: "hi" });
  assert.equal(reply.ok ? "ok" : reply.code, "not_linked");
});

void test("the core plugin guards accept fixture payloads and reject invalid ones", async () => {
  const echo = await fixture("echo");
  const invalid = await fixture("invalid-payload");
  assert.equal(corePlugin.commands.echo.isPayload(echo.request.payload), true);
  assert.equal(corePlugin.commands.echo.isPayload(invalid.request.payload), false);
  assert.equal(corePlugin.commands.capabilities.isPayload((await fixture("capabilities")).request.payload), true);
  assert.equal(corePlugin.commands.capabilities.isPayload({}), false);
});
