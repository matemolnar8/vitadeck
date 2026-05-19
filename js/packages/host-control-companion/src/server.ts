import http from "node:http";
import os from "node:os";

import {
  HOST_CONTROL_COMMANDS,
  HOST_CONTROL_DEFAULT_PORT,
  HOST_CONTROL_PORT_TRIES,
  HOST_CONTROL_PROTOCOL_VERSION,
  HOST_CONTROL_REQUEST_MAX_BYTES,
  type HostCapabilitiesResult,
  type HostControlCommand,
  type HostControlFailureCode,
  type HostControlRequest,
  type HostEchoResult,
  type VitaDeckLanJsonResult,
} from "@vitadeck/sdk/host-control";
import type { HostActionProvider } from "./actions.js";

type JsonResponse = {
  status: number;
  body: VitaDeckLanJsonResult<object>;
};

export type HostControlServer = {
  server: http.Server;
  port: number;
  url: string;
};

function statusText(status: number): string {
  return (
    {
      200: "OK",
      400: "Bad Request",
      404: "Not Found",
      405: "Method Not Allowed",
      413: "Payload Too Large",
      415: "Unsupported Media Type",
      422: "Unprocessable Entity",
      501: "Not Implemented",
    }[status] ?? "Internal Server Error"
  );
}

function failure(status: number, code: HostControlFailureCode, message: string): JsonResponse {
  return { status, body: { ok: false, code, message } };
}

function success<T extends object>(body: T): JsonResponse {
  return { status: 200, body: { ok: true, ...body } };
}

function writeJson(res: http.ServerResponse, response: JsonResponse) {
  const body = JSON.stringify(response.body);
  res.writeHead(response.status, statusText(response.status), {
    "Content-Type": "application/json",
    "Content-Length": Buffer.byteLength(body),
    Connection: "close",
  });
  res.end(body);
}

function contentTypeIsJson(value: string | string[] | undefined): boolean {
  const contentType = Array.isArray(value) ? value[0] : value;
  return contentType?.toLowerCase().split(";")[0]?.trim() === "application/json";
}

async function readBody(req: http.IncomingMessage): Promise<string> {
  let used = 0;
  const chunks: Buffer[] = [];

  for await (const chunk of req) {
    const buffer = Buffer.isBuffer(chunk) ? chunk : Buffer.from(chunk);
    used += buffer.byteLength;
    if (used > HOST_CONTROL_REQUEST_MAX_BYTES) {
      throw new Error("payload_too_large");
    }
    chunks.push(buffer);
  }

  return Buffer.concat(chunks).toString("utf8");
}

function parseRequest(raw: string): HostControlRequest | null {
  try {
    const parsed = JSON.parse(raw) as unknown;
    if (!parsed || typeof parsed !== "object") return null;
    const request = parsed as Record<string, unknown>;
    if (typeof request.command !== "string") return null;
    return { command: request.command, payload: request.payload };
  } catch {
    return null;
  }
}

function isHostControlCommand(command: string): command is HostControlCommand {
  return (HOST_CONTROL_COMMANDS as readonly string[]).includes(command);
}

function commandSupportsPayload(command: HostControlCommand, payload: unknown): boolean {
  if (command === "host.echo") return payload === undefined || (typeof payload === "object" && payload !== null);
  return payload === undefined || (typeof payload === "object" && payload !== null && Object.keys(payload).length === 0);
}

async function dispatch(request: HostControlRequest, actions: HostActionProvider): Promise<JsonResponse> {
  if (!isHostControlCommand(request.command)) {
    return failure(400, "unknown_command", `Unknown Host Control command: ${request.command}`);
  }
  if (!commandSupportsPayload(request.command, request.payload)) {
    return failure(422, "invalid_payload", `Invalid payload for ${request.command}.`);
  }

  if (request.command === "host.capabilities") {
    const commands = new Set<HostControlCommand>(["host.capabilities", "host.echo", ...actions.supportedCommands()]);
    return success<HostCapabilitiesResult>({
      protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
      hostName: os.hostname(),
      platform: process.platform,
      commands: HOST_CONTROL_COMMANDS.filter((command) => commands.has(command)),
    });
  }

  if (request.command === "host.echo") {
    return success<HostEchoResult>({
      payload: request.payload ?? {},
      receivedAt: new Date().toISOString(),
    });
  }

  if (!actions.supportedCommands().includes(request.command)) {
    return failure(501, "unsupported_platform", `${request.command} is not supported on ${process.platform}.`);
  }

  try {
    await actions.run(request.command);
    return success({});
  } catch (error) {
    return failure(501, "command_failed", error instanceof Error ? error.message : "Host action failed.");
  }
}

function createServer(actions: HostActionProvider): http.Server {
  return http.createServer(async (req, res) => {
    if (req.url !== "/v1/command") {
      writeJson(res, failure(404, "not_found", "Host Control endpoint not found."));
      return;
    }
    if (req.method !== "POST") {
      writeJson(res, failure(405, "method_not_allowed", "Use POST /v1/command."));
      return;
    }
    if (!contentTypeIsJson(req.headers["content-type"])) {
      writeJson(res, failure(415, "unsupported_content_type", "Use Content-Type: application/json."));
      return;
    }

    try {
      const body = await readBody(req);
      const request = parseRequest(body);
      if (!request) {
        writeJson(res, failure(400, "malformed_request", "Request body must be JSON with a string command."));
        return;
      }
      writeJson(res, await dispatch(request, actions));
    } catch (error) {
      if (error instanceof Error && error.message === "payload_too_large") {
        writeJson(res, failure(413, "payload_too_large", "Request body exceeds 64KB."));
        return;
      }
      writeJson(res, failure(400, "malformed_request", "Could not read request body."));
    }
  });
}

function detectLanIp(): string {
  for (const interfaces of Object.values(os.networkInterfaces())) {
    for (const entry of interfaces ?? []) {
      if (entry.family === "IPv4" && !entry.internal) return entry.address;
    }
  }
  return "127.0.0.1";
}

function listen(server: http.Server, port: number): Promise<void> {
  return new Promise((resolve, reject) => {
    const onError = (error: Error) => {
      server.off("listening", onListening);
      reject(error);
    };
    const onListening = () => {
      server.off("error", onError);
      resolve();
    };
    server.once("error", onError);
    server.once("listening", onListening);
    server.listen(port, "0.0.0.0");
  });
}

export async function startHostControlServer(actions: HostActionProvider): Promise<HostControlServer> {
  let lastError: unknown;

  for (let offset = 0; offset < HOST_CONTROL_PORT_TRIES; offset++) {
    const port = HOST_CONTROL_DEFAULT_PORT + offset;
    const server = createServer(actions);
    try {
      await listen(server, port);
      return {
        server,
        port,
        url: `http://${detectLanIp()}:${port}`,
      };
    } catch (error) {
      lastError = error;
      server.close();
    }
  }

  const lastMessage = lastError instanceof Error ? ` Last error: ${lastError.message}` : "";
  throw new Error(`Could not bind Host Control Companion on ports 8797-8806.${lastMessage}`);
}
