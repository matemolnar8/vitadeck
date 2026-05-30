import assert from "node:assert/strict";
import { describe, it } from "node:test";

import { cmd, createHostControlGateway, defineHostControlCommands, defineHostControlImplementations } from "./define.js";

describe("createHostControlGateway", () => {
  const registry = defineHostControlCommands({
    "host.echo": cmd<Record<string, unknown> | undefined, { payload: unknown; receivedAt: string }>({
      validatePayload(payload): payload is Record<string, unknown> | undefined {
        return payload === undefined || (typeof payload === "object" && payload !== null);
      },
    }),
    "host.power.sleep_displays": cmd<undefined, Record<string, never>>({
      validatePayload: (payload): payload is undefined => payload === undefined,
    }),
  });

  const implementations = defineHostControlImplementations(registry, {
    "host.echo": {
      default: async (payload) => ({ payload: payload ?? {}, receivedAt: "fixed" }),
    },
    "host.power.sleep_displays": {
      darwin: async () => ({}),
      linux: async () => ({}),
    },
  });

  it("uses a platform-specific implementation when present", async () => {
    const gateway = createHostControlGateway(registry, implementations, {
      platform: "linux",
      hostName: "test",
    });
    assert.deepEqual(gateway.availableCommands(), ["host.echo", "host.power.sleep_displays"]);
    const response = await gateway.handleCommand({ command: "host.power.sleep_displays" });
    assert.equal(response.status, 200);
    assert.equal(response.body.ok, true);
  });

  it("returns unsupported_platform when no implementation exists for this OS", async () => {
    const gateway = createHostControlGateway(registry, implementations, {
      platform: "win32",
      hostName: "test",
    });
    const response = await gateway.handleCommand({ command: "host.power.sleep_displays" });
    assert.equal(response.status, 501);
    assert.equal(response.body.ok, false);
    if (response.body.ok) return;
    assert.equal(response.body.code, "unsupported_platform");
  });

  it("falls back to default implementation", async () => {
    const gateway = createHostControlGateway(registry, implementations, {
      platform: "win32",
      hostName: "test",
    });
    const response = await gateway.handleCommand({ command: "host.echo", payload: { x: 1 } });
    assert.equal(response.status, 200);
    assert.equal(response.body.ok, true);
    if (!response.body.ok) return;
    const body = response.body as unknown as { payload: unknown };
    assert.deepEqual(body.payload, { x: 1 });
  });
});
