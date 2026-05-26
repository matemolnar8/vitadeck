import {
  HOST_CONTROL_PROTOCOL_VERSION,
  type HostControlCommand,
  type HostCapabilitiesResult,
  type HostEchoResult,
} from "@vitadeck/sdk/host-control";

import type { CommandHandler } from "../gateway/types.js";

export function createBuiltinHandlers(listCommands: () => HostControlCommand[]): CommandHandler[] {
  return [
    {
      command: "host.capabilities",
      handle: async (): Promise<HostCapabilitiesResult> => ({
        protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
        hostName: "", // filled by registry wrapper — see createDefaultGateway
        platform: "",
        commands: listCommands(),
      }),
    },
    {
      command: "host.echo",
      validatePayload(payload) {
        if (payload === undefined) return null;
        if (typeof payload === "object" && payload !== null) return null;
        return "Invalid payload for host.echo.";
      },
      handle: async (payload): Promise<HostEchoResult> => ({
        payload: payload ?? {},
        receivedAt: new Date().toISOString(),
      }),
    },
  ];
}

/** Capabilities must report this host's identity; patch after registry is built. */
export function withHostIdentity(
  handlers: CommandHandler[],
  context: { hostName: string; platform: NodeJS.Platform },
): CommandHandler[] {
  return handlers.map((handler) => {
    if (handler.command !== "host.capabilities") return handler;
    return {
      ...handler,
      handle: async (payload, _ctx) => {
        const base = (await handler.handle(payload, _ctx)) as HostCapabilitiesResult;
        return { ...base, hostName: context.hostName, platform: context.platform };
      },
    };
  });
}
