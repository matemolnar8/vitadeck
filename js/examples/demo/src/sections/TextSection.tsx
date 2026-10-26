import { Rect, Stack, useTheme } from "@vitadeck/sdk";
import React from "react";
import { CONTENT_WIDTH, Label, Section } from "../ui";

const ALIGNS = ["left", "center", "right"] as const;

export function TextSection() {
  const { theme } = useTheme();
  return (
    <Section
      title="Text"
      blurb="UTF-8 text with align, word wrap, lineHeight, any fontSize and declared Deck App Fonts."
    >
      <Stack direction="row" gap={10}>
        {ALIGNS.map((align) => (
          <Rect key={align} width={220} height={28} variant="outline" color={theme.outline} borderRadius={4}>
            <Label x={8} y={4} width={204} align={align}>{`align="${align}"`}</Label>
          </Rect>
        ))}
      </Stack>
      <Label fontSize={14} wrap>
        {
          'wrap="word" breaks long lines at spaces inside width, and lineHeight sets the distance between them. This sentence is long enough to need a second line.'
        }
      </Label>
      <Stack direction="row" gap={16} align="end">
        {[12, 16, 24, 32].map((size) => (
          <Label key={size} width={size * 3} fontSize={size}>{`${size}px`}</Label>
        ))}
        <Label width={300} fontSize={18} font="mono" color={theme.success}>
          {'font="mono" 0O1lI {} =>'}
        </Label>
      </Stack>
      <Label fontSize={20}>Árvíztűrő tükörfúrógép</Label>
      <Label fontSize={22}>Emoji: 🎮 🚀 🎉 👍🏽 🇭🇺 ❤️</Label>
      <Stack direction="row" gap={12} align="center">
        <Label width={200} fontSize={20}>
          日本語 中文 한국어
        </Label>
        <Label width={CONTENT_WIDTH - 212} fontSize={13} color={theme.outline}>
          Korean renders as boxes until #95 lands.
        </Label>
      </Stack>
    </Section>
  );
}
