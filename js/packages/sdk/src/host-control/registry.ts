import { HOST_CONTROL_PROTOCOL_VERSION } from "./constants.js";
import { cmd, defineHostControlCommands } from "./define.js";

export type HostCapabilitiesResult = {
  protocolVersion: typeof HOST_CONTROL_PROTOCOL_VERSION;
  hostName: string;
  platform: string;
  commands: readonly string[];
};

export type HostEchoResult = {
  payload: unknown;
  receivedAt: string;
};

function noPayload(payload: unknown): payload is undefined {
  return payload === undefined;
}

function noPayloadOrEmptyObject(payload: unknown): payload is undefined {
  if (payload === undefined) return true;
  return typeof payload === "object" && payload !== null && Object.keys(payload).length === 0;
}

/** Single registry for every Host Control command contract (payload + result types). */
export const hostControlCommands = defineHostControlCommands({
  "host.capabilities": cmd<undefined, HostCapabilitiesResult>({
    validatePayload: noPayload,
    invalidPayloadMessage: "host.capabilities does not accept a payload.",
  }),
  "host.echo": cmd<Record<string, unknown> | undefined, HostEchoResult>({
    validatePayload(payload): payload is Record<string, unknown> | undefined {
      return payload === undefined || (typeof payload === "object" && payload !== null);
    },
  }),
  "host.power.sleep_displays": cmd<undefined, Record<string, never>>({
    validatePayload: noPayloadOrEmptyObject,
  }),
  "host.media.play_pause": cmd<undefined, Record<string, never>>({
    validatePayload: noPayloadOrEmptyObject,
  }),
  "host.media.next": cmd<undefined, Record<string, never>>({
    validatePayload: noPayloadOrEmptyObject,
  }),
  "host.media.previous": cmd<undefined, Record<string, never>>({
    validatePayload: noPayloadOrEmptyObject,
  }),
  "host.media.volume_up": cmd<undefined, Record<string, never>>({
    validatePayload: noPayloadOrEmptyObject,
  }),
  "host.media.volume_down": cmd<undefined, Record<string, never>>({
    validatePayload: noPayloadOrEmptyObject,
  }),
  "host.media.mute": cmd<undefined, Record<string, never>>({
    validatePayload: noPayloadOrEmptyObject,
  }),
});

export type HostControlCommand = (typeof hostControlCommands.commandNames)[number];

export const HOST_CONTROL_COMMANDS = hostControlCommands.commandNames;

export type HostControlPayloads = {
  [K in HostControlCommand]: (typeof hostControlCommands.definitions)[K]["$payload"];
};

export type HostControlResults = {
  [K in HostControlCommand]: (typeof hostControlCommands.definitions)[K]["$result"];
};
