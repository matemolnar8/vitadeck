export const HOST_CONTROL_PROTOCOL_VERSION = 1;
export const HOST_CONTROL_DEFAULT_PORT = 8797;
export const HOST_CONTROL_PORT_TRIES = 10;
export const HOST_CONTROL_REQUEST_MAX_BYTES = 64 * 1024;

export const HOST_CONTROL_COMMANDS = [
  "host.capabilities",
  "host.echo",
  "host.power.sleep_displays",
] as const;

export type HostControlCommand = (typeof HOST_CONTROL_COMMANDS)[number];

export type HostControlRequest = {
  command: string;
  payload?: unknown;
};

export type HostControlFailureCode =
  | "malformed_request"
  | "not_found"
  | "method_not_allowed"
  | "payload_too_large"
  | "unsupported_content_type"
  | "unknown_command"
  | "invalid_payload"
  | "unsupported_platform"
  | "command_failed";

export type VitaDeckLanJsonResult<T extends object = Record<string, never>> =
  | ({ ok: true } & T)
  | { ok: false; code: HostControlFailureCode; message: string };

export type HostCapabilitiesResult = {
  protocolVersion: typeof HOST_CONTROL_PROTOCOL_VERSION;
  hostName: string;
  platform: string;
  commands: HostControlCommand[];
};

export type HostEchoResult = {
  payload: unknown;
  receivedAt: string;
};

export type HostControlPayloads = {
  "host.capabilities": undefined;
  "host.echo": Record<string, unknown> | undefined;
  "host.power.sleep_displays": undefined;
};

export type HostControlResults = {
  "host.capabilities": HostCapabilitiesResult;
  "host.echo": HostEchoResult;
  "host.power.sleep_displays": Record<string, never>;
};

export type HostControlTransportResponse = {
  status: number;
  body: string;
};

export type HostControlTransport = (request: {
  url: string;
  body: string;
  timeoutMs: number;
}) => Promise<HostControlTransportResponse>;

export type HostControlClientOptions = {
  baseUrl?: string;
  timeoutMs?: number;
  transport?: HostControlTransport;
};

export type HostControlClient = {
  command<C extends HostControlCommand>(
    command: C,
    payload?: HostControlPayloads[C],
  ): Promise<VitaDeckLanJsonResult<HostControlResults[C]>>;
  capabilities(): Promise<VitaDeckLanJsonResult<HostCapabilitiesResult>>;
  echo(payload?: Record<string, unknown>): Promise<VitaDeckLanJsonResult<HostEchoResult>>;
  sleepDisplays(): Promise<VitaDeckLanJsonResult>;
};

let nextNativeRequestId = 1;

type HostControlNativeGlobals = {
  nativeGetHostControlBaseUrl?: () => string;
  nativeHostControlHttpPostJson?: (requestId: number, url: string, body: string, timeoutMs: number) => void;
  nativeTakeHostControlHttpResponse?: (requestId: number) => string | null;
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

function nativeTransport({ url, body, timeoutMs }: Parameters<HostControlTransport>[0]) {
  return new Promise<HostControlTransportResponse>((resolve, reject) => {
    const native = nativeGlobals();
    if (!native.nativeHostControlHttpPostJson || !native.nativeTakeHostControlHttpResponse) {
      reject(new Error("Host Control native HTTP transport is unavailable."));
      return;
    }

    const requestId = nextNativeRequestId++;
    native.nativeHostControlHttpPostJson(requestId, url, body, timeoutMs);

    const poll = () => {
      const raw = native.nativeTakeHostControlHttpResponse?.(requestId) ?? null;
      if (raw === null) {
        setTimeout(poll, 16);
        return;
      }

      try {
        const response = JSON.parse(raw) as HostControlTransportResponse & { error?: string };
        if (response.error) {
          reject(new Error(response.error));
          return;
        }
        resolve({ status: response.status, body: response.body });
      } catch {
        reject(new Error("Host Control native HTTP transport returned malformed JSON."));
      }
    };
    setTimeout(poll, 16);
  });
}

function parseResult<T extends object>(response: HostControlTransportResponse): VitaDeckLanJsonResult<T> {
  const parsed = JSON.parse(response.body) as unknown;
  if (!parsed || typeof parsed !== "object" || typeof (parsed as { ok?: unknown }).ok !== "boolean") {
    throw new Error("Host Control response did not use VitaDeck LAN JSON Result.");
  }
  return parsed as VitaDeckLanJsonResult<T>;
}

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
    const body = payload === undefined ? JSON.stringify({ command: commandName }) : JSON.stringify({ command: commandName, payload });
    const response = await transport({
      url: `${baseUrl}/v1/command`,
      body,
      timeoutMs,
    });
    return parseResult<HostControlResults[C]>(response);
  }

  return {
    command,
    capabilities: () => command("host.capabilities"),
    echo: (payload) => command("host.echo", payload),
    sleepDisplays: () => command("host.power.sleep_displays"),
  };
}
