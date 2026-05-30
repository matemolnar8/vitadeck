import http from "node:http";
import os from "node:os";

import {
  HOST_CONTROL_DEFAULT_PORT,
  HOST_CONTROL_PORT_TRIES,
  HOST_CONTROL_REQUEST_MAX_BYTES,
  hostControlFailure,
  type HostControlRequest,
  type createHostControlGateway,
} from "@vitadeck/sdk/host-control";

export type HostControlServer = {
  server: http.Server;
  port: number;
  url: string;
};

type HostControlGateway = ReturnType<typeof createHostControlGateway>;

export type StartHostControlServerOptions = {
  callbackHost?: string;
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

function writeJson(res: http.ServerResponse, status: number, body: object) {
  const serialized = JSON.stringify(body);
  res.writeHead(status, statusText(status), {
    "Content-Type": "application/json",
    "Content-Length": Buffer.byteLength(serialized),
    Connection: "close",
  });
  res.end(serialized);
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

export function detectLanIp(): string {
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

function createHttpServer(gateway: HostControlGateway): http.Server {
  return http.createServer(async (req, res) => {
    if (req.url !== "/v1/command") {
      const response = hostControlFailure(404, "not_found", "Host Control endpoint not found.");
      writeJson(res, response.status, response.body);
      return;
    }
    if (req.method !== "POST") {
      const response = hostControlFailure(405, "method_not_allowed", "Use POST /v1/command.");
      writeJson(res, response.status, response.body);
      return;
    }
    if (!contentTypeIsJson(req.headers["content-type"])) {
      const response = hostControlFailure(
        415,
        "unsupported_content_type",
        "Use Content-Type: application/json.",
      );
      writeJson(res, response.status, response.body);
      return;
    }

    try {
      const body = await readBody(req);
      const request = parseRequest(body);
      if (!request) {
        const response = hostControlFailure(
          400,
          "malformed_request",
          "Request body must be JSON with a string command.",
        );
        writeJson(res, response.status, response.body);
        return;
      }
      const response = await gateway.handleCommand(request);
      if (request.command === "host.echo") {
        console.log(`[host-control] handled ${request.command}`);
      }
      writeJson(res, response.status, response.body);
    } catch (error) {
      if (error instanceof Error && error.message === "payload_too_large") {
        const response = hostControlFailure(413, "payload_too_large", "Request body exceeds 64KB.");
        writeJson(res, response.status, response.body);
        return;
      }
      const response = hostControlFailure(400, "malformed_request", "Could not read request body.");
      writeJson(res, response.status, response.body);
    }
  });
}

export async function startHostControlServer(
  gateway: HostControlGateway,
  options: StartHostControlServerOptions = {},
): Promise<HostControlServer> {
  const host = options.callbackHost?.trim() || detectLanIp();
  let lastError: unknown;

  for (let offset = 0; offset < HOST_CONTROL_PORT_TRIES; offset++) {
    const port = HOST_CONTROL_DEFAULT_PORT + offset;
    const server = createHttpServer(gateway);
    try {
      await listen(server, port);
      return {
        server,
        port,
        url: `http://${host}:${port}`,
      };
    } catch (error) {
      lastError = error;
      server.close();
    }
  }

  const lastMessage = lastError instanceof Error ? ` Last error: ${lastError.message}` : "";
  throw new Error(`Could not bind Host Control Companion on ports 8797-8806.${lastMessage}`);
}
