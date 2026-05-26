import {
  HOST_CONTROL_COMMANDS,
  type HostControlCommand,
  type HostControlRequest,
} from "@vitadeck/sdk/host-control";

import { hostControlFailure, hostControlSuccess, type HostControlJsonResponse } from "../lan-json.js";
import type { CommandHandler, CommandHandlerContext, HostControlGateway } from "./types.js";

function isKnownCommand(command: string): command is HostControlCommand {
  return (HOST_CONTROL_COMMANDS as readonly string[]).includes(command);
}

function validatePayload(handler: CommandHandler, payload: unknown): string | null {
  if (handler.validatePayload) return handler.validatePayload(payload);
  if (payload === undefined) return null;
  if (typeof payload === "object" && payload !== null && Object.keys(payload).length === 0) return null;
  return "Invalid payload";
}

export function createHostControlGateway(
  handlers: CommandHandler[],
  context: CommandHandlerContext,
): HostControlGateway {
  const byCommand = new Map<HostControlCommand, CommandHandler>();
  for (const handler of handlers) {
    byCommand.set(handler.command, handler);
  }

  const availableCommands = (): HostControlCommand[] =>
    HOST_CONTROL_COMMANDS.filter((command) => byCommand.has(command));

  async function handleCommand(request: HostControlRequest): Promise<HostControlJsonResponse> {
    if (!isKnownCommand(request.command)) {
      return hostControlFailure(400, "unknown_command", `Unknown Host Control command: ${request.command}`);
    }

    const handler = byCommand.get(request.command);
    if (!handler) {
      return hostControlFailure(
        501,
        "unsupported_platform",
        `${request.command} is not supported on ${context.platform}.`,
      );
    }

    const payloadError = validatePayload(handler, request.payload);
    if (payloadError) {
      return hostControlFailure(422, "invalid_payload", payloadError);
    }

    try {
      const result = await handler.handle(request.payload, context);
      return hostControlSuccess(result);
    } catch (error) {
      return hostControlFailure(
        501,
        "command_failed",
        error instanceof Error ? error.message : "Host action failed.",
      );
    }
  }

  return { handleCommand, availableCommands };
}
