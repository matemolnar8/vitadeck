import { Rect, Screen, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React from "react";

const USER_MESSAGE =
  "Can you summarize what VitaDeck is in two sentences? I want something short enough to test wrapping on the right side of the screen.";

const AGENT_MESSAGE =
  "VitaDeck runs custom interactive deck-style interfaces on a PlayStation Vita.\n\nDeck Apps are built with React against the VitaDeck Runtime API, then packaged and loaded by the runtime.";

export default function ChatDeckApp() {
  const { theme } = useTheme();
  const { x: insetX, y: insetY, width: cw } = insetContent();

  const bubbleWidth = 420;
  const bubblePadding = 16;
  const textWidth = bubbleWidth - bubblePadding * 2;
  const fontSize = 22;
  const lineHeight = 28;
  const userLineCount = 5;
  const agentLineCount = 8;
  const textBlockHeight = (lineCount: number) => {
    if (lineCount <= 0) return 0;
    if (lineCount === 1) return fontSize + 4;
    return (lineCount - 1) * lineHeight + (fontSize + 4);
  };
  const bubbleHeight = (lines: number) => bubblePadding * 2 + textBlockHeight(lines);
  const userBubbleX = insetX + cw - bubbleWidth;
  const userBubbleY = insetY + 24;
  const userBubbleHeight = bubbleHeight(userLineCount);
  const agentBubbleX = insetX;
  const agentBubbleY = userBubbleY + userBubbleHeight + 32;
  const agentBubbleHeight = bubbleHeight(agentLineCount);

  return (
    <Screen>
      <Rect
        x={userBubbleX}
        y={userBubbleY}
        width={bubbleWidth}
        height={userBubbleHeight}
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

      <Rect
        x={agentBubbleX}
        y={agentBubbleY}
        width={bubbleWidth}
        height={agentBubbleHeight}
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
          {AGENT_MESSAGE}
        </Text>
      </Rect>
    </Screen>
  );
}
