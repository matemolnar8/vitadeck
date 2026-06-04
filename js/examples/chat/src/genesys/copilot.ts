import { genesysConfig } from "./config";
import { apiBaseUrl, genesysApiHeaders, readJsonResponse } from "./http";
import { getAccessToken } from "./token";

const POLL_INTERVAL_MS = 2000;
const MAX_POLL_ATTEMPTS = 60;

export type CopilotMessageContent = {
  contentType?: string;
  markdown?: { text?: string };
};

export type CopilotMessage = {
  id?: string;
  dateSent?: string;
  originatingEntity?: string;
  session?: { id?: string; selfUri?: string };
  content?: CopilotMessageContent[];
};

export type CopilotSendResponse = {
  id?: string;
  session?: { id?: string };
};

export type CopilotMessageListing = {
  entities?: CopilotMessage[];
};

function delay(ms: number): Promise<void> {
  return new Promise((resolve) => setTimeout(resolve, ms));
}

function extractMarkdownText(message: CopilotMessage): string {
  for (const block of message.content ?? []) {
    if (block.contentType === "Markdown" && block.markdown?.text) {
      return block.markdown.text;
    }
  }
  return "";
}

export async function sendCopilotMessage(
  accessToken: string,
  content: string,
): Promise<CopilotSendResponse> {
  const res = await fetch(`${apiBaseUrl()}/api/v2/apps/agentic/copilots/messages`, {
    method: "POST",
    headers: genesysApiHeaders(accessToken),
    body: JSON.stringify({
      source: "QuickAction",
      content,
      navigationContext: genesysConfig.navigationContext,
      preferredLanguage: genesysConfig.preferredLanguage,
      timezone: genesysConfig.timezone,
    }),
  });
  return readJsonResponse<CopilotSendResponse>(res);
}

export async function getSessionMessages(
  accessToken: string,
  sessionId: string,
): Promise<CopilotMessageListing> {
  const res = await fetch(
    `${apiBaseUrl()}/api/v2/apps/agentic/copilots/sessions/${sessionId}/messages?pageSize=25`,
    {
      method: "GET",
      headers: genesysApiHeaders(accessToken),
    },
  );
  return readJsonResponse<CopilotMessageListing>(res);
}

function findLatestAssistantMessage(messages: CopilotMessage[]): CopilotMessage | undefined {
  return messages.find((m) => m.originatingEntity === "Assistant");
}

export async function sendCopilotMessageAndWaitForReply(content: string): Promise<string> {
  const accessToken = await getAccessToken();
  const sent = await sendCopilotMessage(accessToken, content);
  const sessionId = sent.session?.id;
  if (!sessionId) {
    throw new Error("Copilot response missing session id");
  }

  for (let attempt = 0; attempt < MAX_POLL_ATTEMPTS; attempt++) {
    await delay(POLL_INTERVAL_MS);
    const listing = await getSessionMessages(accessToken, sessionId);
    const assistant = findLatestAssistantMessage(listing.entities ?? []);
    if (assistant) {
      const text = extractMarkdownText(assistant);
      return text || "(empty Copilot reply)";
    }
  }

  throw new Error("Timed out waiting for Copilot reply");
}
