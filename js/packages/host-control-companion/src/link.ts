import {
  HOST_CONTROL_LINK_PATH,
  HOST_CONTROL_LINK_RETRY_MS,
  HOST_CONTROL_PROTOCOL_VERSION,
} from "@vitadeck/sdk/host-control";

function trimSlash(url: string): string {
  return url.replace(/\/+$/, "");
}

async function postLink(vitaUrl: string, callbackUrl: string, hostName: string): Promise<void> {
  const linkUrl = `${trimSlash(vitaUrl)}${HOST_CONTROL_LINK_PATH}`;
  const response = await fetch(linkUrl, {
    method: "POST",
    headers: { "Content-Type": "application/json", Accept: "application/json" },
    body: JSON.stringify({
      protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
      hostName,
      callbackUrl,
    }),
  });
  const body = (await response.text()) as string;
  if (!response.ok) {
    throw new Error(`Host Control Link failed (${response.status}): ${body}`);
  }
  const parsed = JSON.parse(body) as { ok?: boolean; linked?: boolean };
  if (!parsed.ok || !parsed.linked) {
    throw new Error(`Host Control Link rejected: ${body}`);
  }
}

function sleep(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

export async function linkToVitaWithRetry(
  vitaUrl: string,
  callbackUrl: string,
  hostName: string,
): Promise<void> {
  for (;;) {
    try {
      await postLink(vitaUrl, callbackUrl, hostName);
      console.log(`[host-control] linked to Vita (${callbackUrl})`);
      return;
    } catch (error) {
      const message = error instanceof Error ? error.message : String(error);
      console.log(`[host-control] link retry: ${message}`);
      await sleep(HOST_CONTROL_LINK_RETRY_MS);
    }
  }
}
