import type { HostControlCommand, HostControlPayloads, HostControlResults } from "./registry.js";
import { hostControlCommands } from "./registry.js";
import type { HostControlTransport, HostControlTransportResponse, VitaDeckLanJsonResult } from "./lan-json.js";

export type { HostControlTransport, HostControlTransportResponse, VitaDeckLanJsonResult };

export type HostControlClientOptions = {
  baseUrl?: string;
  timeoutMs?: number;
  transport?: HostControlTransport;
};

type HostControlNativeGlobals = {
  nativeGetHostControlBaseUrl?: () => string;
  nativeHostControlFetch?: (url: string, body: string, timeoutMs: number) => Promise<string>;
};

function nativeGlobals(): HostControlNativeGlobals {
  return globalThis as unknown as HostControlNativeGlobals;
}

function trimTrailingSlash(value: string): string {
  return value.replace(/\/+$/, "");
}

function getDefaultBaseUrl(): string {
  const native = nativeGlobals();
  return native.nativeGetHostControlBaseUrl?.() ?? "";
}

type NativeTransportEnvelope = HostControlTransportResponse & { error?: string };

async function nativeTransport({ url, body, timeoutMs }: Parameters<HostControlTransport>[0]) {
  const native = nativeGlobals();
  if (!native.nativeHostControlFetch) {
    throw new Error("Host Control native HTTP transport is unavailable.");
  }

  const raw = await native.nativeHostControlFetch(url, body, timeoutMs);
  let envelope: NativeTransportEnvelope;
  try {
    envelope = JSON.parse(raw) as NativeTransportEnvelope;
  } catch {
    throw new Error("Host Control native HTTP transport returned malformed JSON.");
  }

  if (envelope.error) {
    throw new Error(envelope.error);
  }

  return { status: envelope.status, body: envelope.body };
}

function parseResult<T extends object>(response: HostControlTransportResponse): VitaDeckLanJsonResult<T> {
  const parsed = JSON.parse(response.body) as unknown;
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
  const timeoutMs = options.timeoutMs ?? 5000;
  const transport = options.transport ?? nativeTransport;
  const configuredBaseUrl = options.baseUrl ?? getDefaultBaseUrl();
  const baseUrl = trimTrailingSlash(configuredBaseUrl);

  async function command<C extends HostControlCommand>(
    commandName: C,
    payload?: HostControlPayloads[C],
  ): Promise<VitaDeckLanJsonResult<HostControlResults[C]>> {
    if (!baseUrl) throw new Error("Host Control Console Address is not configured.");
    const body =
      payload === undefined
        ? JSON.stringify({ command: commandName })
        : JSON.stringify({ command: commandName, payload });
    const response = await transport({
      url: `${baseUrl}/v1/command`,
      body,
      timeoutMs,
    });
    return parseResult<HostControlResults[C]>(response);
  }

  return { command };
}

void hostControlCommands;
