import type { HostControlCommand, HostControlPayloads, HostControlResults } from "./registry.js";
import { hostControlCommands } from "./registry.js";
import type { VitaDeckLanJsonResult } from "./lan-json.js";

export type HostControlClientOptions = {
  timeoutMs?: number;
};

type HostControlNativeGlobals = {
  nativeHostControlCommand?: (
    command: string,
    payloadJson: string,
    timeoutMs: number,
  ) => Promise<string>;
};

function nativeGlobals(): HostControlNativeGlobals {
  return globalThis as unknown as HostControlNativeGlobals;
}

function parseResult<T extends object>(raw: string): VitaDeckLanJsonResult<T> {
  const parsed = JSON.parse(raw) as unknown;
  if (!parsed || typeof parsed !== "object" || typeof (parsed as { ok?: unknown }).ok !== "boolean") {
    throw new Error("Host Control response did not use VitaDeck LAN JSON Result.");
  }
  return parsed as VitaDeckLanJsonResult<T>;
}

export type HostControlClient = {
  command<C extends HostControlCommand>(
    command: C,
    ...args: HostControlPayloads[C] extends undefined ? [] : [payload: HostControlPayloads[C]]
  ): Promise<VitaDeckLanJsonResult<HostControlResults[C]>>;
};

export function createHostControlClient(options: HostControlClientOptions = {}): HostControlClient {
  const timeoutMs = options.timeoutMs ?? 10000;

  async function command<C extends HostControlCommand>(
    commandName: C,
    payload?: HostControlPayloads[C],
  ): Promise<VitaDeckLanJsonResult<HostControlResults[C]>> {
    const native = nativeGlobals();
    if (!native.nativeHostControlCommand) {
      throw new Error("Host Control native bridge is unavailable.");
    }

    const payloadJson =
      payload === undefined ? "null" : JSON.stringify(payload);

    const raw = await native.nativeHostControlCommand(commandName, payloadJson, timeoutMs);
    return parseResult<HostControlResults[C]>(raw);
  }

  return { command };
}

export const hostControl = createHostControlClient();

void hostControlCommands;
