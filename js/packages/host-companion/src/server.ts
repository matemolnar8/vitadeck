import { createServer, type IncomingMessage, type ServerResponse } from "node:http";
import type { AddressInfo } from "node:net";
import { networkInterfaces } from "node:os";
import {
  HOST_CONTROL_PROTOCOL_VERSION,
  HOST_CONTROL_WIRE_ERRORS,
  isRecord,
  isWireErrorCode,
  type HostControlWireErrorCode,
  type ImplementedHostPlugin,
} from "@vitadeck/sdk/host-control";
import { implementCorePlugin } from "./core";
import { createEventLog, EVENT_WAIT_MS } from "./events";

export const MAX_REQUEST_BYTES = 64 * 1024;

export type HostCompanionOptions = {
  port: number;
  host?: string;
  name: string;
  platform?: string;
  /** Loaded after `core`, which is always served. */
  plugins?: readonly ImplementedHostPlugin[];
  /** Linked Vita IPs, read per request so a `link` from another process applies at once. */
  vitas?: () => Promise<readonly string[]>;
  remoteAddress?: (request: IncomingMessage) => string | undefined;
  log?: (line: string) => void;
  /** How long `core.events` waits for an event; shortened in tests. */
  eventWaitMs?: number;
};

export type RunningHostCompanion = {
  port: number;
  /** Replaces every plugin but `core`, stopping the old ones and starting the new, or throws and keeps them. */
  setPlugins: (plugins: readonly ImplementedHostPlugin[]) => void;
  close: () => Promise<void>;
};

type Reply = { status: number; label: string; outcome: string; body: object };

function fail(label: string, code: HostControlWireErrorCode, message: string): Reply {
  return { status: HOST_CONTROL_WIRE_ERRORS[code], label, outcome: code, body: { ok: false, code, message } };
}

export function normalizeAddress(address: string): string {
  return address.startsWith("::ffff:") ? address.slice("::ffff:".length) : address;
}

function isLoopback(address: string): boolean {
  return address === "::1" || address.startsWith("127.");
}

function ownAddresses(): string[] {
  return Object.values(networkInterfaces()).flatMap((list) => list?.map((info) => info.address) ?? []);
}

function readBody(request: IncomingMessage): Promise<Buffer | undefined> {
  return new Promise((resolve, reject) => {
    const chunks: Buffer[] = [];
    let size = 0;
    // Drains past the limit so the 413 reaches a client that is still sending.
    request.on("data", (chunk: Buffer) => {
      size += chunk.length;
      if (size <= MAX_REQUEST_BYTES) chunks.push(chunk);
    });
    request.on("end", () => {
      resolve(size <= MAX_REQUEST_BYTES ? Buffer.concat(chunks) : undefined);
    });
    request.on("error", reject);
  });
}

/** Matched by shape, since a Host Plugin Package throws its own bundled copy of `HostCommandError`. */
function wireErrorCode(error: unknown): HostControlWireErrorCode | undefined {
  return error instanceof Error && error.name === "HostCommandError" && "code" in error && isWireErrorCode(error.code)
    ? error.code
    : undefined;
}

function parseJson(text: string): unknown {
  try {
    return JSON.parse(text);
  } catch {
    return undefined;
  }
}

export async function startHostCompanion(options: HostCompanionOptions): Promise<RunningHostCompanion> {
  const {
    name,
    platform = process.platform,
    vitas = () => Promise.resolve([]),
    remoteAddress = (request: IncomingMessage) => request.socket.remoteAddress,
    log = (line) => {
      console.log(line);
    },
  } = options;

  const events = createEventLog(options.eventWaitMs ?? EVENT_WAIT_MS);
  let plugins = new Map<string, ImplementedHostPlugin>();
  const core = implementCorePlugin(name, platform, () => [...plugins.values()], events);

  // Each plugin's lifecycle runs while it is loaded; an emit after its stop is dropped.
  function startPlugins(loaded: readonly ImplementedHostPlugin[]): () => void {
    let running = true;
    const stops = loaded.flatMap((plugin) => {
      try {
        const stop = plugin.start?.((event, data) => {
          if (running) events.emit(plugin.id, event, data);
        });
        return stop ? [stop] : [];
      } catch (error) {
        log(`Plugin ${plugin.id} failed to start: ${error instanceof Error ? error.message : String(error)}`);
        return [];
      }
    });
    return () => {
      running = false;
      for (const stop of stops) {
        try {
          stop();
        } catch (error) {
          log(`A plugin failed to stop: ${error instanceof Error ? error.message : String(error)}`);
        }
      }
    };
  }

  let stopRunning = startPlugins([]);

  function setPlugins(next: readonly ImplementedHostPlugin[]): void {
    const byId = new Map([[core.id, core]]);
    for (const plugin of next) {
      if (byId.has(plugin.id)) throw new Error(`Two loaded plugins share the id ${plugin.id}.`);
      byId.set(plugin.id, plugin);
    }
    stopRunning();
    plugins = byId;
    stopRunning = startPlugins(next);
  }
  setPlugins(options.plugins ?? []);

  async function isAllowed(peer: string): Promise<boolean> {
    const linked = await vitas();
    return linked.length === 0 || isLoopback(peer) || linked.includes(peer) || ownAddresses().includes(peer);
  }

  async function reply(request: IncomingMessage, peer: string): Promise<Reply> {
    const route = `${request.method ?? "?"} ${request.url ?? "?"}`;
    if (!(await isAllowed(peer))) {
      // Discards the body unbuffered, so a peer that is not linked costs no memory.
      request.resume();
      return fail(route, "forbidden", "This Host Companion only answers the VitaDeck it is linked with.");
    }
    const body = await readBody(request);
    // Every status comes from HOST_CONTROL_WIRE_ERRORS, so an unknown route is an unknown command.
    if (new URL(request.url ?? "/", "http://companion").pathname !== "/command") {
      return fail(route, "unknown_command", "This Host Companion serves only POST /command.");
    }
    if (request.method !== "POST") return fail(route, "malformed_request", "Send commands with POST /command.");
    const contentType = request.headers["content-type"]?.split(";")[0]?.trim().toLowerCase();
    if (contentType !== "application/json") {
      return fail(route, "malformed_request", "Send commands with Content-Type: application/json.");
    }
    if (body === undefined) {
      return fail(route, "payload_too_large", `A command request may be at most ${MAX_REQUEST_BYTES} bytes.`);
    }

    const envelope = parseJson(body.toString("utf8"));
    if (!isRecord(envelope) || typeof envelope.protocolVersion !== "number") {
      return fail(route, "malformed_request", "The request body is not a Host Control command.");
    }
    if (envelope.protocolVersion !== HOST_CONTROL_PROTOCOL_VERSION) {
      return fail(
        route,
        "unsupported_protocol_version",
        `This Host Companion speaks Host Control protocol version ${HOST_CONTROL_PROTOCOL_VERSION}.`,
      );
    }
    if (typeof envelope.plugin !== "string" || typeof envelope.command !== "string") {
      return fail(route, "malformed_request", "The request body is not a Host Control command.");
    }

    const label = `${envelope.plugin}.${envelope.command}`;
    const plugin = plugins.get(envelope.plugin);
    if (!plugin) return fail(label, "unknown_plugin", `This Host Companion has no plugin ${envelope.plugin}.`);
    const command = plugin.commands.get(envelope.command);
    if (!command) return fail(label, "unknown_command", `Plugin ${plugin.id} has no command ${envelope.command}.`);
    if (!command.isPayload(envelope.payload)) return fail(label, "invalid_payload", `Invalid payload for ${label}.`);

    try {
      const result = await command.run(envelope.payload);
      return { status: 200, label, outcome: "ok", body: { ...result, ok: true } };
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      return fail(label, wireErrorCode(error) ?? "command_failed", message);
    }
  }

  async function answer(request: IncomingMessage, response: ServerResponse): Promise<void> {
    const started = performance.now();
    const peer = normalizeAddress(remoteAddress(request) ?? "unknown");
    let result: Reply;
    try {
      result = await reply(request, peer);
    } catch (error) {
      result = fail("?", "command_failed", String(error));
    }
    const text = JSON.stringify(result.body);
    response.writeHead(result.status, {
      "Content-Type": "application/json",
      "Content-Length": Buffer.byteLength(text),
      Connection: "close",
    });
    response.end(text);
    const ms = Math.round(performance.now() - started);
    log(`${new Date().toISOString()} ${peer} ${result.label} ${result.outcome} ${ms}ms`);
  }

  const server = createServer((request, response) => {
    void answer(request, response);
  });

  await new Promise<void>((resolve, reject) => {
    server.once("error", (error: NodeJS.ErrnoException) => {
      reject(
        error.code === "EADDRINUSE"
          ? new Error(`Port ${options.port} is already in use. Stop the other program or choose another --port.`)
          : error,
      );
    });
    server.listen(options.port, options.host ?? "0.0.0.0", () => {
      resolve();
    });
  });

  return {
    port: (server.address() as AddressInfo).port,
    setPlugins,
    close: () =>
      new Promise((resolve, reject) => {
        events.close();
        stopRunning();
        if (!server.listening) {
          resolve();
          return;
        }
        server.close((error) => {
          if (error) reject(error);
          else resolve();
        });
      }),
  };
}
