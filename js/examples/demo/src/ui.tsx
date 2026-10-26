import { Rect, Stack, Text, useTheme, type Color, type FontName, type TextProps } from "@vitadeck/sdk";
import React, { type ReactNode } from "react";

export const CONTENT_WIDTH = 680;

type LabelProps = {
  children: string;
  width?: number;
  fontSize?: number;
  /** Reserve this many lines and wrap at word boundaries. */
  lines?: number;
  color?: Color;
  font?: FontName;
  align?: TextProps["align"];
  x?: number;
  y?: number;
};

/** A `Stack` does not lay out `Text`, so each line of copy sits in a transparent `Rect` sized to it. */
export function Label({
  children,
  width = CONTENT_WIDTH,
  fontSize = 16,
  lines = 1,
  color,
  font,
  align,
  x,
  y,
}: LabelProps) {
  const { theme } = useTheme();
  const lineHeight = Math.round(fontSize * 1.4);
  return (
    <Rect x={x} y={y} width={width} height={lines * lineHeight}>
      <Text
        x={0}
        y={0}
        width={width}
        fontSize={fontSize}
        lineHeight={lineHeight}
        color={color ?? theme.text}
        font={font}
        align={align}
        wrap={lines > 1 ? "word" : "none"}
      >
        {children}
      </Text>
    </Rect>
  );
}

type SectionProps = {
  title: string;
  blurb: string;
  blurbLines?: number;
  children?: ReactNode;
};

export function Section({ title, blurb, blurbLines = 2, children }: SectionProps) {
  const { theme } = useTheme();
  return (
    <Stack x={16} y={14} width={CONTENT_WIDTH} gap={12}>
      <Label fontSize={24} color={theme.accent}>
        {title}
      </Label>
      <Label fontSize={15} lines={blurbLines} color={theme.outline}>
        {blurb}
      </Label>
      {children}
    </Stack>
  );
}
