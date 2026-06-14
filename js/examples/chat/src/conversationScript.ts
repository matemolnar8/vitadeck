/** First message in a new Copilot session (no sessionId). */
export const OPENING_MESSAGE = "Hello, where am I?";

export const FOLLOW_UPS = [
  "I'm on a PS Vita with no keyboard — does Copilot count D-pad taps as digital channel handle time?",
  "Am I in a queue, or just waiting for my Vita's WiFi to wake up from sleep mode?",
  "Can you suggest a wrap-up code for 'customer device was a PlayStation Vita'?",
  "I Runtime-Uploaded this Deck App over LAN — is that a supported omnichannel touchpoint?",
  "If I enable AudioHook, are you streaming my Vita's fan noise or just the OLED hum?",
  "Is scrolling chat history with the analog stick billable ACW?",
  "Predictive Routing: please match me with an agent who also runs enterprise SaaS on 512MB RAM.",
  "Summarize this chat for after-call work — keep it under 960px wide, that's my whole screen.",
  "Knowledge Optimizer, surface articles about Genesys Cloud on a handheld from 2011.",
  "Virtual Agent, can you take the next message? I need both hands for the shoulder buttons.",
  "Does Journey Management map the path from LiveArea bubble tap to this Copilot session?",
  "Which queue handles contacts who opened PureCloud on a screen smaller than a sticky note?",
] as const;

export function pendingUserMessage(turn: number, followUpIndex: number): string {
  if (turn === 0) return OPENING_MESSAGE;
  const count = FOLLOW_UPS.length;
  const index = ((followUpIndex % count) + count) % count;
  return FOLLOW_UPS[index]!;
}
