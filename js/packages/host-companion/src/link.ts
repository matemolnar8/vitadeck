import { createSocket } from "node:dgram";
import {
  HOST_CONTROL_PROTOCOL_VERSION,
  corePlugin,
  createHostClient,
  isRecord,
  type HostControlLinkRequest,
} from "@vitadeck/sdk/host-control";
import { loadConfig, saveConfig } from "./config";

export const DEFAULT_VITA_LINK_PORT = 8787;
const LINK_TIMEOUT_MS = 5000;

export type LinkOptions = {
  configPath: string;
  /** The port `start` listens on. Defaults to the config's. */
  port?: number;
  /** The IP to advertise. Defaults to the config's, then to the interface routed to the Vita. */
  advertise?: string;
};

export type LinkOutcome = { ok: boolean; message: string };

export function parseVitaAddress(address: string): { host: string; port: number } | undefined {
  let url: URL;
  try {
    url = new URL(`http://${address}`);
  } catch {
    return undefined;
  }
  if (url.pathname !== "/" || url.search !== "" || url.username !== "" || url.hash !== "") return undefined;
  return { host: url.hostname, port: url.port === "" ? DEFAULT_VITA_LINK_PORT : Number(url.port) };
}

/** Connecting a UDP socket sends nothing, but makes the OS pick the local interface that routes to `host`. */
async function route(host: string, port: number): Promise<{ local: string; remote: string }> {
  const socket = createSocket("udp4");
  try {
    await new Promise<void>((resolve, reject) => {
      socket.once("error", reject);
      socket.connect(port, host, () => {
        resolve();
      });
    });
    return { local: socket.address().address, remote: socket.remoteAddress().address };
  } finally {
    socket.close();
  }
}

async function postLink(url: string, request: HostControlLinkRequest): Promise<{ status: number; body: unknown }> {
  const response = await fetch(url, {
    method: "POST",
    headers: { "Content-Type": "application/json" },
    body: JSON.stringify(request),
    signal: AbortSignal.timeout(LINK_TIMEOUT_MS),
  });
  const text = await response.text();
  try {
    return { status: response.status, body: JSON.parse(text) };
  } catch {
    return { status: response.status, body: undefined };
  }
}

/** Tells the VitaDeck at `vitaAddress` to send Host Control commands to this computer's running companion. */
export async function linkVita(vitaAddress: string, options: LinkOptions): Promise<LinkOutcome> {
  const target = parseVitaAddress(vitaAddress);
  if (!target)
    return { ok: false, message: `${vitaAddress} is not an address like 192.168.1.177 or 192.168.1.177:8787.` };

  const config = await loadConfig(options.configPath);
  const port = options.port ?? config.port;

  const self = createHostClient(corePlugin, { url: `http://127.0.0.1:${port}/`, timeoutMs: 2000 });
  const capabilities = await self.capabilities();
  if (!capabilities.ok) {
    return {
      ok: false,
      message: `No Host Companion answers on port ${port} (${capabilities.message}). Run \`vitadeck-host-companion start\` first.`,
    };
  }

  let routed: { local: string; remote: string };
  try {
    routed = await route(target.host, target.port);
  } catch (error) {
    return { ok: false, message: `Cannot reach ${target.host}: ${String(error)}` };
  }

  const advertised = options.advertise ?? config.advertise ?? routed.local;
  const request: HostControlLinkRequest = {
    protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
    id: config.id,
    name: config.name,
    url: `http://${advertised}:${port}/`,
  };
  const linkUrl = `http://${routed.remote}:${target.port}/link`;

  let answer: { status: number; body: unknown };
  try {
    answer = await postLink(linkUrl, request);
  } catch (error) {
    const reason = error instanceof Error && error.cause instanceof Error ? error.cause.message : String(error);
    return {
      ok: false,
      message: `Could not reach VitaDeck at ${linkUrl} (${reason}). Check that Upload and link is on in the VitaDeck Shell, and that the address matches the one it shows.`,
    };
  }

  const { body } = answer;
  if (!isRecord(body) || typeof body.ok !== "boolean") {
    return { ok: false, message: `${linkUrl} answered HTTP ${answer.status} without a Host Control result.` };
  }
  if (!body.ok) {
    const code = typeof body.code === "string" ? body.code : "unknown";
    const message = typeof body.message === "string" ? body.message : "No reason given.";
    return { ok: false, message: `VitaDeck refused the link (${code}): ${message}` };
  }

  if (!config.vitas.includes(routed.remote)) {
    await saveConfig(options.configPath, { ...config, vitas: [...config.vitas, routed.remote] });
  }
  return {
    ok: true,
    message: `Linked "${config.name}" with VitaDeck at ${routed.remote}. It will send commands to ${request.url}`,
  };
}
