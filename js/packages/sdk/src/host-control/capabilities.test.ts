import assert from "node:assert/strict";
import { describe, it } from "node:test";

import { HOST_CONTROL_PROTOCOL_VERSION } from "./constants.js";
import { availableHostCommands, isHostCommandAvailable } from "./capabilities.js";
import type { HostCapabilitiesResult } from "./registry.js";
import type { VitaDeckLanJsonResult } from "./lan-json.js";

describe("host control capabilities helpers", () => {
  it("isHostCommandAvailable returns true when command is listed", () => {
    const caps = {
      ok: true,
      protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
      hostName: "pc",
      platform: "linux",
      commands: ["host.echo", "host.capabilities"],
    } satisfies VitaDeckLanJsonResult<HostCapabilitiesResult>;
    assert.equal(isHostCommandAvailable(caps, "host.echo"), true);
    assert.equal(isHostCommandAvailable(caps, "host.power.sleep_displays"), false);
  });

  it("isHostCommandAvailable returns false on failure", () => {
    assert.equal(
      isHostCommandAvailable({ ok: false, code: "command_failed", message: "x" }, "host.echo"),
      false,
    );
  });

  it("availableHostCommands returns commands or empty", () => {
    assert.deepEqual(
      availableHostCommands({
        ok: true,
        protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
        hostName: "pc",
        platform: "linux",
        commands: ["host.echo"],
      }),
      ["host.echo"],
    );
    assert.deepEqual(
      availableHostCommands({ ok: false, code: "malformed_request", message: "bad" }),
      [],
    );
  });
});
