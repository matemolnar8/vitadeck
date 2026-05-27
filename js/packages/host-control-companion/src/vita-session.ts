import {
  HOST_CONTROL_POLL_PATH,
  HOST_CONTROL_POLL_TIMEOUT_MS,
  HOST_CONTROL_RESULT_PATH,
  type HostControlRequest,
} from "@vitadeck/sdk/host-control";

import { createDefaultGateway } from "./commands/index.js";

export type PollWorkItem = {
  requestId: number;
  command: string;
  payload?: unknown;
};

type HostControlGateway = ReturnType<typeof createDefaultGateway>;

function trimSlash(url: string): string {
  return url.replace(/\/+$/, "");
}

async function fetchText(url: string, init: RequestInit, timeoutMs: number): Promise<{ status: number; body: string }> {
  const controller = new AbortController();
  const timer = setTimeout(() => controller.abort(), timeoutMs);
  try {
    const response = await fetch(url, { ...init, signal: controller.signal });
    const body = await response.text();
    return { status: response.status, body };
  } finally {
    clearTimeout(timer);
  }
}

export async function runVitaSession(vitaUrl: string, gateway: HostControlGateway = createDefaultGateway()): Promise<never> {
  const base = trimSlash(vitaUrl);
  const pollUrl = `${base}${HOST_CONTROL_POLL_PATH}`;
  const resultUrl = `${base}${HOST_CONTROL_RESULT_PATH}`;

  while (true) {
    try {
      const poll = await fetchText(
        pollUrl,
        { method: "GET", headers: { Accept: "application/json" } },
        HOST_CONTROL_POLL_TIMEOUT_MS + 5000,
      );

      if (poll.status === 204 || poll.body.length === 0) {
        continue;
      }

      if (poll.status !== 200) {
        await sleep(1000);
        continue;
      }

      const work = JSON.parse(poll.body) as PollWorkItem;
      if (!work || typeof work.requestId !== "number" || typeof work.command !== "string") {
        continue;
      }

      const request: HostControlRequest = {
        command: work.command,
        payload: work.payload,
      };
      const response = await gateway.handleCommand(request);
      console.log(`[host-control] handled ${work.command} requestId=${work.requestId}`);
      const payload = {
        requestId: work.requestId,
        result: response.body,
      };

      await fetchText(
        resultUrl,
        {
          method: "POST",
          headers: { "Content-Type": "application/json" },
          body: JSON.stringify(payload),
        },
        10_000,
      );
    } catch {
      await sleep(2000);
    }
  }
}

function sleep(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms));
}
