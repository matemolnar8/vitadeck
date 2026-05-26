import { execFile } from "node:child_process";
import { promisify } from "node:util";

import {
  HOST_CONTROL_PROTOCOL_VERSION,
  defineHostControlImplementations,
  hostControlCommands,
  type HostControlImplementationsFor,
} from "@vitadeck/sdk/host-control";

const execFileAsync = promisify(execFile);

/**
 * Per-command platform implementations. Add OS keys on the same command — not grouped by OS file.
 * `default` runs when the current platform has no specific handler.
 */
export const hostControlImplementations = defineHostControlImplementations(hostControlCommands, {
  "host.capabilities": {
    default: async (_payload, context) => ({
      protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
      hostName: context.hostName,
      platform: context.platform,
      commands: context.availableCommands(),
    }),
  },
  "host.echo": {
    default: async (payload) => ({
      payload: payload ?? {},
      receivedAt: new Date().toISOString(),
    }),
  },
  "host.power.sleep_displays": {
    darwin: async () => {
      await execFileAsync("pmset", ["displaysleepnow"]);
      return {};
    },
    // Example: add linux when a safe impl exists
    // linux: async () => { ... return {}; },
  },
} satisfies HostControlImplementationsFor<typeof hostControlCommands.definitions>);
