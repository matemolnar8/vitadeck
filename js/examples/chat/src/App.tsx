import { Button, Rect, Screen, Scroll, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useState } from "react";
import { isGenesysConfigured } from "./genesys/config";
import { sendCopilotMessageAndWaitForReply } from "./genesys/copilot";

const USER_MESSAGE = "Hello, where am I?";

const AGENT_IDLE = "Press Send to message Genesys Cloud Copilot.";

const NOT_CONFIGURED_MESSAGE =
  "Genesys Cloud is not configured.\n\nSet credentials in js/examples/chat/src/genesys/genesys.local.ts (see genesys.local.example.ts).";

function estimateLineCount(text: string, charsPerLine: number): number {
  if (!text) return 1;
  let lines = 0;
  for (const paragraph of text.split("\n")) {
    lines += Math.max(1, Math.ceil(paragraph.length / charsPerLine));
  }
  return lines;
}

export default function ChatDeckApp() {
  const { theme } = useTheme();
  const inset = insetContent();

  const scrollPadding = 16;
  const scrollGap = 24;
  const contentWidth = inset.width - scrollPadding * 2;

  const configured = isGenesysConfigured();
  const [showSendButton, setShowSendButton] = useState(configured);
  const [sending, setSending] = useState(false);
  const [agentMessage, setAgentMessage] = useState(configured ? AGENT_IDLE : NOT_CONFIGURED_MESSAGE);

  const sendMessage = async () => {
    if (sending || !configured) return;

    setShowSendButton(false);
    setSending(true);
    setAgentMessage("Waiting for Copilot...");

    try {
      const reply = await sendCopilotMessageAndWaitForReply(USER_MESSAGE);
      setAgentMessage(reply);
    } catch (err: unknown) {
      setAgentMessage(`Genesys Cloud Copilot error:\n\n${String(err)}`);
    } finally {
      setSending(false);
    }
  };

  const bubbleWidth = 600;
  const bubblePadding = 16;
  const textWidth = bubbleWidth - bubblePadding * 2;
  const fontSize = 22;
  const lineHeight = 28;
  const charsPerLine = Math.max(12, Math.floor(textWidth / (fontSize * 0.55)));

  const sendButtonWidth = 88;
  const sendButtonHeight = 40;

  const userLineCount = estimateLineCount(USER_MESSAGE, charsPerLine);
  const agentLineCount = estimateLineCount(agentMessage, charsPerLine);

  const textBlockHeight = (lineCount: number) => {
    if (lineCount <= 0) return 0;
    if (lineCount === 1) return fontSize + 4;
    return (lineCount - 1) * lineHeight + (fontSize + 4);
  };
  const bubbleHeight = (lines: number) => bubblePadding * 2 + textBlockHeight(lines);

  return (
    <Screen>
      <Scroll
        x={inset.x}
        y={inset.y}
        width={inset.width}
        height={inset.height}
        gap={scrollGap}
        padding={scrollPadding}
      >
        <Rect
          x={contentWidth - bubbleWidth}
          y={0}
          width={bubbleWidth}
          height={bubbleHeight(userLineCount)}
          color={theme.primary}
          borderRadius={12}
        >
          <Text
            x={bubblePadding}
            y={bubblePadding}
            width={textWidth}
            align="right"
            wrap="word"
            fontSize={fontSize}
            lineHeight={lineHeight}
            color={theme.navText}
          >
            {USER_MESSAGE}
          </Text>
        </Rect>

        {showSendButton ? (
          <Button
            x={contentWidth - sendButtonWidth}
            y={0}
            width={sendButtonWidth}
            height={sendButtonHeight}
            label={sending ? "..." : "Send"}
            onPress={() => {
              if (!sending) void sendMessage();
            }}
            color={theme.accent}
            textColor={theme.navText}
            borderRadius={0.12}
          />
        ) : null}

        <Rect
          x={0}
          y={0}
          width={bubbleWidth}
          height={bubbleHeight(agentLineCount)}
          color={theme.surface}
          borderRadius={12}
        >
          <Text
            x={bubblePadding}
            y={bubblePadding}
            width={textWidth}
            align="left"
            wrap="word"
            font="chatMono"
            fontSize={fontSize}
            lineHeight={lineHeight}
            color={theme.text}
          >
            {agentMessage}
          </Text>
        </Rect>
      </Scroll>
    </Screen>
  );
}
