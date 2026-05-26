import assert from "node:assert/strict";
import { describe, it } from "node:test";

import type { CommandHandler } from "./types.js";
import { createHostControlGateway } from "./registry.js";

const context = { platform: "linux" as const, hostName: "test-host" };

describe("HostControlGateway", () => {
  const handlers: CommandHandler[] = [
    {
      command: "host.echo",
      validatePayload(payload) {
        if (payload === undefined) return null;
        if (typeof payload === "object" && payload !== null) return null;
        return "Invalid payload for host.echo.";
      },
      handle: async (payload) => ({ payload: payload ?? {}, receivedAt: "2020-01-01T00:00:00.000Z" }),
    },
  ];

  const gateway = createHostControlGateway(handlers, context);

  it("returns unknown_command for strings outside HOST_CONTROL_COMMANDS", async () => {
    const response = await gateway.handleCommand({ command: "host.nope" });
    assert.equal(response.status, 400);
    assert.equal(response.body.ok, false);
    if (response.body.ok) return;
    assert.equal(response.body.code, "unknown_command");
  });

  it("returns unsupported_platform when command is known but not registered", async () => {
    const response = await gateway.handleCommand({ command: "host.power.sleep_displays" });
    assert.equal(response.status, 501);
    assert.equal(response.body.ok, false);
    if (response.body.ok) return;
    assert.equal(response.body.code, "unsupported_platform");
  });

  it("dispatches a registered handler", async () => {
    const response = await gateway.handleCommand({ command: "host.echo", payload: { x: 1 } });
    assert.equal(response.status, 200);
    assert.equal(response.body.ok, true);
    if (!response.body.ok) return;
    const body = response.body as unknown as { payload: unknown };
    assert.deepEqual(body.payload, { x: 1 });
  });

  it("lists only registered commands in availableCommands", () => {
    assert.deepEqual(gateway.availableCommands(), ["host.echo"]);
  });
});
