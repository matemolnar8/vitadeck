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
  sessionId?: string,
): Promise<CopilotSendResponse> {
  const body: Record<string, unknown> = {
    source: "QuickAction",
    content,
    navigationContext: genesysConfig.navigationContext,
    preferredLanguage: genesysConfig.preferredLanguage,
    timezone: genesysConfig.timezone,
  };
  if (sessionId) {
    body.sessionId = sessionId;
  }

  const res = await fetch(`${apiBaseUrl()}/api/v2/apps/agentic/copilots/messages`, {
    method: "POST",
    headers: genesysApiHeaders(accessToken),
    body: JSON.stringify(body),
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
  const assistants = messages.filter((m) => m.originatingEntity === "Assistant");
  if (assistants.length === 0) return undefined;

  return assistants.reduce((latest, message) => {
    if (!latest) return message;
    const latestTime = latest.dateSent ? Date.parse(latest.dateSent) : 0;
    const messageTime = message.dateSent ? Date.parse(message.dateSent) : 0;
    return messageTime >= latestTime ? message : latest;
  });
}

export type CopilotReply = {
  reply: string;
  sessionId: string;
};

export async function sendCopilotMessageAndWaitForReply(
  content: string,
  sessionId?: string,
): Promise<CopilotReply> {
  const accessToken = await getAccessToken();

  let baselineAssistantId: string | undefined;
  if (sessionId) {
    const existing = await getSessionMessages(accessToken, sessionId);
    baselineAssistantId = findLatestAssistantMessage(existing.entities ?? [])?.id;
  }

  const sent = await sendCopilotMessage(accessToken, content, sessionId);
  const activeSessionId = sent.session?.id ?? sessionId;
  if (!activeSessionId) {
    throw new Error("Copilot response missing session id");
  }

  for (let attempt = 0; attempt < MAX_POLL_ATTEMPTS; attempt++) {
    await delay(POLL_INTERVAL_MS);
    const listing = await getSessionMessages(accessToken, activeSessionId);
    const assistant = findLatestAssistantMessage(listing.entities ?? []);
    if (!assistant) continue;
    if (baselineAssistantId && assistant.id === baselineAssistantId) continue;

    const text = extractMarkdownText(assistant);
    return {
      reply: text || "(empty Copilot reply)",
      sessionId: activeSessionId,
    };
  }

  throw new Error("Timed out waiting for Copilot reply");
}
