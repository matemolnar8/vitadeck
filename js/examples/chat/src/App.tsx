import { Rect, Screen, Scroll, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React from "react";

type ChatMessage = {
  side: "user" | "agent";
  text: string;
  /** Wrapped line count at the bubble text width; drives bubble height. */
  lines: number;
};

const MESSAGES: ChatMessage[] = [
  {
    side: "user",
    text: "Hey! Can you explain what VitaDeck is in a couple of sentences?",
    lines: 3,
  },
  {
    side: "agent",
    text: "VitaDeck runs custom interactive deck-style interfaces on a PlayStation Vita. Deck Apps are built with React against the VitaDeck Runtime API.",
    lines: 6,
  },
  {
    side: "user",
    text: "Nice. How do I get my Deck App onto the device?",
    lines: 2,
  },
  {
    side: "agent",
    text: "Package it with the SDK, then upload the archive from the Upload screen. The runtime loads the package and renders your component.",
    lines: 5,
  },
  {
    side: "user",
    text: "Does scrolling work with the D-pad as well?",
    lines: 2,
  },
  {
    side: "agent",
    text: "Yes. Focus the chat list, then press up or down on the D-pad or keyboard. The analog stick and mouse wheel scroll smoothly too.",
    lines: 5,
  },
];

export default function ChatDeckApp() {
  const { theme } = useTheme();
  const inset = insetContent();

  const scrollPadding = 16;
  const scrollGap = 24;
  const contentWidth = inset.width - scrollPadding * 2;

  const bubbleWidth = 420;
  const bubblePadding = 16;
  const textWidth = bubbleWidth - bubblePadding * 2;
  const fontSize = 22;
  const lineHeight = 28;
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
        {MESSAGES.map((message, index) => {
          const isUser = message.side === "user";
          return (
            <Rect
              key={index}
              x={isUser ? contentWidth - bubbleWidth : 0}
              y={0}
              width={bubbleWidth}
              height={bubbleHeight(message.lines)}
              color={isUser ? theme.primary : theme.surface}
              borderRadius={12}
            >
              <Text
                x={bubblePadding}
                y={bubblePadding}
                width={textWidth}
                align={isUser ? "right" : "left"}
                wrap="word"
                font={isUser ? "default" : "chatMono"}
                fontSize={fontSize}
                lineHeight={lineHeight}
                color={isUser ? theme.navText : theme.text}
              >
                {message.text}
              </Text>
            </Rect>
          );
        })}
      </Scroll>
    </Screen>
  );
}
