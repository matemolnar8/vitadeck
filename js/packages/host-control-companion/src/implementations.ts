import { execFile } from "node:child_process";
import { promisify } from "node:util";

import {
  HOST_CONTROL_PROTOCOL_VERSION,
  defineHostControlImplementations,
  hostControlCommands,
  type HostControlImplementationsFor,
} from "@vitadeck/sdk/host-control";

import { pressMediaKey, type MediaKeyAction } from "./media-keys.js";

const execFileAsync = promisify(execFile);

function mediaCommand(action: MediaKeyAction) {
  return {
    linux: async () => {
      await pressMediaKey("linux", action);
      return {};
    },
    darwin: async () => {
      await pressMediaKey("darwin", action);
      return {};
    },
  };
}

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
  },
  "host.media.play_pause": mediaCommand("play_pause"),
  "host.media.next": mediaCommand("next"),
  "host.media.previous": mediaCommand("previous"),
  "host.media.volume_up": mediaCommand("volume_up"),
  "host.media.volume_down": mediaCommand("volume_down"),
  "host.media.mute": mediaCommand("mute"),
} satisfies HostControlImplementationsFor<typeof hostControlCommands.definitions>);
