import { Stack, Text, useTheme, type Color, type FontName, type TextProps } from "@vitadeck/sdk";
import React, { type ReactNode } from "react";

export const CONTENT_WIDTH = 680;

type LabelProps = {
  children: string;
  width?: number;
  fontSize?: number;
  /** Wrap at word boundaries inside `width`. */
  wrap?: boolean;
  color?: Color;
  font?: FontName;
  align?: TextProps["align"];
  x?: number;
  y?: number;
};

export function Label({
  children,
  width = CONTENT_WIDTH,
  fontSize = 16,
  wrap = false,
  color,
  font,
  align,
  x,
  y,
}: LabelProps) {
  const { theme } = useTheme();
  return (
    <Text
      x={x}
      y={y}
      width={width}
      fontSize={fontSize}
      lineHeight={Math.round(fontSize * 1.4)}
      color={color ?? theme.text}
      font={font}
      align={align}
      wrap={wrap ? "word" : "none"}
    >
      {children}
    </Text>
  );
}

type SectionProps = {
  title: string;
  blurb: string;
  children?: ReactNode;
};

export function Section({ title, blurb, children }: SectionProps) {
  const { theme } = useTheme();
  return (
    <Stack x={16} y={14} width={CONTENT_WIDTH} gap={12}>
      <Label fontSize={24} color={theme.accent}>
        {title}
      </Label>
      <Label fontSize={15} wrap color={theme.outline}>
        {blurb}
      </Label>
      {children}
    </Stack>
  );
}
