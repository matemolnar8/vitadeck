import { Rect, Stack, useTheme, type Theme } from "@vitadeck/sdk";
import React from "react";
import { Label, Section } from "../ui";

const SWATCHES = [
  "primary",
  "accent",
  "success",
  "warning",
  "danger",
  "surface",
  "surfaceAlt",
  "outline",
] as const satisfies ReadonlyArray<keyof Theme>;

export function RectSection() {
  const { theme } = useTheme();
  return (
    <Section
      title="Screen, theme and Rect"
      blurb="Screen fills the 960×544 display with theme.background. Rect draws a fill or an outline with any borderRadius, and useTheme() hands out the palette."
    >
      <Stack direction="row" gap={16} align="center">
        <Rect width={96} height={64} color={theme.primary} />
        <Rect width={96} height={64} color={theme.primary} borderRadius={12} />
        <Rect width={96} height={64} variant="outline" color={theme.accent} />
        <Rect width={96} height={64} variant="outline" color={theme.accent} borderRadius={24} />
        <Rect width={96} height={64} color={theme.surface} borderColor={theme.success} borderRadius={8} />
        <Rect width={64} height={64} color={theme.warning} borderRadius={32} />
      </Stack>
      <Label fontSize={13} color={theme.outline}>
        fill · fill r=12 · outline · outline r=24 · fill + borderColor · circle (r = half the size)
      </Label>
      <Stack direction="row" gap={8}>
        {SWATCHES.map((name) => (
          <Stack key={name} gap={4} align="center">
            <Rect width={76} height={40} color={theme[name]} borderColor={theme.outline} borderRadius={6} />
            <Label width={76} fontSize={12} align="center">
              {name}
            </Label>
          </Stack>
        ))}
      </Stack>
    </Section>
  );
}
