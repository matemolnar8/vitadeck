import type { HostControlCommand, HostControlRequest } from "@vitadeck/sdk/host-control";

import type { HostControlJsonResponse } from "../lan-json.js";

export type CommandHandlerContext = {
  platform: NodeJS.Platform;
  hostName: string;
};

/** One registered command. Return value is merged into a success VitaDeck LAN JSON Result (without `ok`). */
export type CommandHandler = {
  command: HostControlCommand;
  validatePayload?(payload: unknown): string | null;
  handle(payload: unknown, context: CommandHandlerContext): Promise<object>;
};

export type HostControlGateway = {
  handleCommand(request: HostControlRequest): Promise<HostControlJsonResponse>;
  availableCommands(): HostControlCommand[];
};
