import assert from "node:assert/strict";
import { describe, it } from "node:test";
import { createHostControlClient } from "./client.js";

describe("createHostControlClient", () => {
  it("rejects when nativeHostControlFetch is missing", async () => {
    const prevFetch = (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch;
    const prevBase = (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl;
    delete (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch;
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = () =>
      "http://127.0.0.1:8797";
    const client = createHostControlClient();
    await assert.rejects(
      () => client.command("host.echo", { x: 1 }),
      (error: unknown) =>
        error instanceof Error && error.message === "Host Control native bridge is unavailable.",
    );
    (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch = prevFetch;
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = prevBase;
  });

  it("rejects when Host Callback URL is not configured", async () => {
    const prevFetch = (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch;
    const prevBase = (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl;
    (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch = async () =>
      JSON.stringify({ status: 200, body: JSON.stringify({ ok: true, receivedAt: "t", payload: {} }) });
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = () => "";
    const client = createHostControlClient();
    await assert.rejects(
      () => client.command("host.echo", {}),
      (error: unknown) => error instanceof Error && error.message === "Host Control Unavailable",
    );
    (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch = prevFetch;
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = prevBase;
  });

  it("reads Host Callback URL when it becomes available after client creation", async () => {
    const prevFetch = (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch;
    const prevBase = (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl;
    let linked = false;
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = () =>
      linked ? "http://127.0.0.1:8797" : "";
    (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch = async () =>
      JSON.stringify({
        status: 200,
        body: JSON.stringify({ ok: true, payload: {}, receivedAt: "2026-01-01T00:00:00.000Z" }),
      });
    const client = createHostControlClient();
    await assert.rejects(
      () => client.command("host.echo", {}),
      (error: unknown) => error instanceof Error && error.message === "Host Control Unavailable",
    );
    linked = true;
    const result = await client.command("host.echo", { ping: 1 });
    assert.equal(result.ok, true);
    (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch = prevFetch;
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = prevBase;
  });

  it("parses a successful native JSON response", async () => {
    const prevFetch = (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch;
    const prevBase = (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl;
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = () =>
      "http://127.0.0.1:8797";
    (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch = async () =>
      JSON.stringify({
        status: 200,
        body: JSON.stringify({ ok: true, payload: {}, receivedAt: "2026-01-01T00:00:00.000Z" }),
      });
    const client = createHostControlClient();
    const result = await client.command("host.echo", { ping: 1 });
    assert.equal(result.ok, true);
    (globalThis as { nativeHostControlFetch?: unknown }).nativeHostControlFetch = prevFetch;
    (globalThis as { nativeGetHostControlBaseUrl?: unknown }).nativeGetHostControlBaseUrl = prevBase;
  });
});
