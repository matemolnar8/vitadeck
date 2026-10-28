import type { HostCommandMap, HostPlugin, PayloadOf, ResultOf } from "./plugin";
import { isRecord } from "./plugin";
import type { HostCommandRequest, HostFailure, HostResult } from "./protocol";
import { HOST_CONTROL_PROTOCOL_VERSION, isWireErrorCode } from "./protocol";

/** A computer whose Host Companion linked with this VitaDeck, as listed on the Shell Host Control Screen. */
export type LinkedHost = { id: string; name: string; url: string };

export type HostTransport = (
  url: string,
  init: { method: "POST"; headers: Record<string, string>; body: string },
) => Promise<{ status: number; text(): Promise<string> }>;

export type HostClientOptions = {
  /** The linked host to call, by id. Defaults to the most recently linked one. */
  hostId?: string;
  /** Calls this companion URL instead of a linked host. */
  url?: string;
  transport?: HostTransport;
  timeoutMs?: number;
};

type HostMethod<P, R extends object> = [P] extends [undefined]
  ? () => Promise<HostResult<R>>
  : (payload: P) => Promise<HostResult<R>>;

export type HostClient<C extends HostCommandMap> = {
  readonly [K in keyof C]: HostMethod<PayloadOf<C[K]>, ResultOf<C[K]>>;
};

const DEFAULT_TIMEOUT_MS = 5000;

// Declared for the host in the runtime's runtime-native.d.ts; kept off the author-visible globals.
const native = globalThis as unknown as { nativeGetHostControlLinks?: () => string };

function isLinkedHost(value: unknown): value is LinkedHost {
  return (
    isRecord(value) && typeof value.id === "string" && typeof value.name === "string" && typeof value.url === "string"
  );
}

/** The linked hosts, most recently linked first. Reads VitaDeck on every call, so a new link shows up at once. */
export function linkedHosts(): LinkedHost[] {
  const raw = native.nativeGetHostControlLinks?.() ?? "";
  if (raw === "") return [];
  try {
    const parsed: unknown = JSON.parse(raw);
    return Array.isArray(parsed) ? parsed.filter(isLinkedHost) : [];
  } catch {
    return [];
  }
}

function failure(code: HostFailure["code"], message: string): HostFailure {
  return { ok: false, code, message };
}

function resolveUrl(options: HostClientOptions): string | HostFailure {
  if (options.url !== undefined) return options.url;
  const hosts = linkedHosts();
  const target = options.hostId === undefined ? hosts[0] : hosts.find((host) => host.id === options.hostId);
  if (target) return target.url;
  return failure(
    "not_linked",
    options.hostId === undefined
      ? "No computer is linked. Link one from Host Control in the VitaDeck Shell."
      : "That computer is no longer linked.",
  );
}

function parseResult(text: string): HostResult<object> {
  let parsed: unknown;
  try {
    parsed = JSON.parse(text);
  } catch {
    return failure("bad_response", "The Host Companion did not answer with JSON.");
  }
  if (!isRecord(parsed) || typeof parsed.ok !== "boolean") {
    return failure("bad_response", "The Host Companion answered without an ok field.");
  }
  if (parsed.ok) return parsed as { ok: true };
  const message = typeof parsed.message === "string" ? parsed.message : "The command failed.";
  return failure(isWireErrorCode(parsed.code) ? parsed.code : "command_failed", message);
}

const defaultTransport: HostTransport = (url, init) => fetch(url, init);

async function send(
  plugin: string,
  command: string,
  payload: unknown,
  options: HostClientOptions,
): Promise<HostResult<object>> {
  const url = resolveUrl(options);
  if (typeof url !== "string") return url;

  const request: HostCommandRequest = { protocolVersion: HOST_CONTROL_PROTOCOL_VERSION, plugin, command, payload };
  const transport = options.transport ?? defaultTransport;
  const timeoutMs = options.timeoutMs ?? DEFAULT_TIMEOUT_MS;

  // The Vita fetch has no AbortSignal, so a timed-out command may still run on the computer. Never retried.
  let timer: ReturnType<typeof setTimeout> | undefined;
  const timeout = new Promise<HostFailure>((resolve) => {
    timer = setTimeout(
      () => resolve(failure("timeout", `The Host Companion did not answer within ${timeoutMs} ms.`)),
      timeoutMs,
    );
  });
  const exchange = (async (): Promise<HostResult<object>> => {
    try {
      const response = await transport(`${url.endsWith("/") ? url : `${url}/`}command`, {
        method: "POST",
        headers: { "Content-Type": "application/json" },
        body: JSON.stringify(request),
      });
      return parseResult(await response.text());
    } catch (error) {
      return failure("unreachable", `Could not reach the Host Companion: ${String(error)}`);
    }
  })();

  try {
    return await Promise.race([exchange, timeout]);
  } finally {
    clearTimeout(timer);
  }
}

/**
 * A typed client for one Host Control Plugin. Every method resolves to a `HostResult` and never rejects for a
 * Host Control failure. Without `hostId` or `url` it calls the most recently linked host, read at call time.
 */
export function createHostClient<C extends HostCommandMap>(
  plugin: HostPlugin<C>,
  options: HostClientOptions = {},
): HostClient<C> {
  const methods: Record<string, (payload?: unknown) => Promise<HostResult<object>>> = {};
  for (const name of Object.keys(plugin.commands)) {
    methods[name] = (payload) => send(plugin.id, name, payload, options);
  }
  return methods as HostClient<C>;
}
