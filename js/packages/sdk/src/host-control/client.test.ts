import assert from "node:assert/strict";
import { describe, it } from "node:test";
import { createHostControlClient } from "./client.js";

describe("createHostControlClient", () => {
  it("rejects when nativeHostControlCommand is missing", async () => {
    const prev = (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand;
    delete (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand;
    const client = createHostControlClient();
    await assert.rejects(
      () => client.command("host.echo", { x: 1 }),
      (error: unknown) => error instanceof Error && error.message === "Host Control native bridge is unavailable.",
    );
    (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand = prev;
  });

  it("rejects when nativeHostControlCommand is not callable", async () => {
    const prev = (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand;
    (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand = "not-a-function";
    const client = createHostControlClient();
    await assert.rejects(
      () => client.command("host.echo", {}),
      (error: unknown) => error instanceof Error && error.message === "Host Control native bridge is unavailable.",
    );
    (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand = prev;
  });

  it("parses a successful native JSON response", async () => {
    const prev = (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand;
    (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand = async () =>
      JSON.stringify({ ok: true, payload: {}, receivedAt: "2026-01-01T00:00:00.000Z" });
    const client = createHostControlClient();
    const result = await client.command("host.echo", { ping: 1 });
    assert.equal(result.ok, true);
    (globalThis as { nativeHostControlCommand?: unknown }).nativeHostControlCommand = prev;
  });
});
