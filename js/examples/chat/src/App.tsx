import { Button, Rect, Screen, Scroll, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useState } from "react";
import { FOLLOW_UPS, pendingUserMessage } from "./conversationScript";
import { isGenesysConfigured } from "./genesys/config";
import { sendCopilotMessageAndWaitForReply } from "./genesys/copilot";

const AGENT_IDLE = "Press Send to message Genesys Cloud Copilot.";

const NOT_CONFIGURED_MESSAGE =
  "Genesys Cloud is not configured.\n\nSet credentials in js/examples/chat/src/genesys/genesys.local.ts (see genesys.local.example.ts).";

type ChatMessage = {
  id: string;
  role: "user" | "agent";
  text: string;
};

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
  const [messages, setMessages] = useState<ChatMessage[]>(
    configured ? [] : [{ id: "not-configured", role: "agent", text: NOT_CONFIGURED_MESSAGE }],
  );
  const [turn, setTurn] = useState(0);
  const [followUpIndex, setFollowUpIndex] = useState(0);
  const [sessionId, setSessionId] = useState<string | undefined>();
  const [showSendButton, setShowSendButton] = useState(configured);
  const [sending, setSending] = useState(false);

  const selectedUserMessage = configured ? pendingUserMessage(turn, followUpIndex) : "";
  const showFollowUpPicker = turn > 0;

  const sendMessage = async () => {
    if (sending || !configured) return;

    const userText = selectedUserMessage;
    const messageTurn = turn;

    setShowSendButton(false);
    setSending(true);
    setMessages((prev) => [...prev, { id: `u-${messageTurn}`, role: "user", text: userText }]);
    setMessages((prev) => [...prev, { id: `a-${messageTurn}-waiting`, role: "agent", text: "Waiting for Copilot..." }]);

    try {
      const { reply, sessionId: nextSessionId } = await sendCopilotMessageAndWaitForReply(
        userText,
        sessionId,
      );
      setSessionId(nextSessionId);
      setMessages((prev) =>
        prev.map((m) => (m.id === `a-${messageTurn}-waiting` ? { ...m, text: reply } : m)),
      );
      setTurn((t) => t + 1);
      setShowSendButton(true);
    } catch (err: unknown) {
      setMessages((prev) =>
        prev.map((m) =>
          m.id === `a-${messageTurn}-waiting`
            ? { ...m, text: `Genesys Cloud Copilot error:\n\n${String(err)}` }
            : m,
        ),
      );
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
  const navButtonWidth = 48;
  const navButtonHeight = 40;
  const navGap = 8;

  const textBlockHeight = (lineCount: number) => {
    if (lineCount <= 0) return 0;
    if (lineCount === 1) return fontSize + 4;
    return (lineCount - 1) * lineHeight + (fontSize + 4);
  };
  const bubbleHeight = (lines: number) => bubblePadding * 2 + textBlockHeight(lines);
  const bubbleRadiusPx = 12;

  const renderBubble = (message: ChatMessage) => {
    const isUser = message.role === "user";
    const lineCount = estimateLineCount(message.text, charsPerLine);

    return (
      <Rect
        key={message.id}
        x={isUser ? contentWidth - bubbleWidth : 0}
        y={0}
        width={bubbleWidth}
        height={bubbleHeight(lineCount)}
        color={isUser ? theme.primary : theme.surface}
        borderRadius={bubbleRadiusPx}
      >
        <Text
          x={bubblePadding}
          y={bubblePadding}
          width={textWidth}
          align={isUser ? "right" : "left"}
          wrap="word"
          font={isUser ? "chatMono" : undefined}
          fontSize={fontSize}
          lineHeight={lineHeight}
          color={isUser ? theme.navText : theme.text}
        >
          {message.text}
        </Text>
      </Rect>
    );
  };

  const pendingLineCount = estimateLineCount(selectedUserMessage, charsPerLine);
  const pendingBubbleHeight = bubbleHeight(pendingLineCount);
  const pickerRowHeight = Math.max(navButtonHeight, pendingBubbleHeight);
  const pendingBubbleY = (pickerRowHeight - pendingBubbleHeight) / 2;
  const navButtonY = (pickerRowHeight - navButtonHeight) / 2;
  const pickerGroupWidth =
    navButtonWidth + navGap + bubbleWidth + navGap + navButtonWidth;
  const pickerGroupLeft = contentWidth - pickerGroupWidth;
  const pickerBubbleX = pickerGroupLeft + navButtonWidth + navGap;
  const pickerRightNavX = pickerBubbleX + bubbleWidth + navGap;

  const stepFollowUp = (delta: number) => {
    setFollowUpIndex((index) => {
      const count = FOLLOW_UPS.length;
      return (index + delta + count) % count;
    });
  };

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
        {messages.length === 0 ? (
          <Rect
            x={0}
            y={0}
            width={bubbleWidth}
            height={bubbleHeight(estimateLineCount(AGENT_IDLE, charsPerLine))}
            color={theme.surface}
            borderRadius={bubbleRadiusPx}
          >
            <Text
              x={bubblePadding}
              y={bubblePadding}
              width={textWidth}
              align="left"
              wrap="word"
              fontSize={fontSize}
              lineHeight={lineHeight}
              color={theme.text}
            >
              {AGENT_IDLE}
            </Text>
          </Rect>
        ) : (
          messages.map(renderBubble)
        )}

        {showSendButton && configured ? (
          <>
            {showFollowUpPicker ? (
              <Rect x={0} y={0} width={contentWidth} height={pickerRowHeight}>
                <Button
                  x={pickerGroupLeft}
                  y={navButtonY}
                  width={navButtonWidth}
                  height={navButtonHeight}
                  label="←"
                  onPress={() => stepFollowUp(-1)}
                  color={theme.surfaceAlt}
                  textColor={theme.text}
                  borderRadius={bubbleRadiusPx}
                />
                <Rect
                  x={pickerBubbleX}
                  y={pendingBubbleY}
                  width={bubbleWidth}
                  height={pendingBubbleHeight}
                  color={theme.primary}
                  borderRadius={bubbleRadiusPx}
                >
                  <Text
                    x={bubblePadding}
                    y={bubblePadding}
                    width={textWidth}
                    align="right"
                    wrap="word"
                    font="chatMono"
                    fontSize={fontSize}
                    lineHeight={lineHeight}
                    color={theme.navText}
                  >
                    {selectedUserMessage}
                  </Text>
                </Rect>
                <Button
                  x={pickerRightNavX}
                  y={navButtonY}
                  width={navButtonWidth}
                  height={navButtonHeight}
                  label="→"
                  onPress={() => stepFollowUp(1)}
                  color={theme.surfaceAlt}
                  textColor={theme.text}
                  borderRadius={bubbleRadiusPx}
                />
              </Rect>
            ) : (
              <Rect
                x={contentWidth - bubbleWidth}
                y={0}
                width={bubbleWidth}
                height={pendingBubbleHeight}
                color={theme.primary}
                borderRadius={bubbleRadiusPx}
              >
                <Text
                  x={bubblePadding}
                  y={bubblePadding}
                  width={textWidth}
                  align="right"
                  wrap="word"
                  font="chatMono"
                  fontSize={fontSize}
                  lineHeight={lineHeight}
                  color={theme.navText}
                >
                  {selectedUserMessage}
                </Text>
              </Rect>
            )}

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
              borderRadius={bubbleRadiusPx}
            />
          </>
        ) : null}
      </Scroll>
    </Screen>
  );
}
