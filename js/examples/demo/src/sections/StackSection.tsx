import { Rect, Stack, useTheme } from "@vitadeck/sdk";
import React from "react";
import { Label, Section } from "../ui";

const ALIGNS = ["start", "center", "end"] as const;

export function StackSection() {
  const { theme } = useTheme();
  return (
    <Section
      title="Stack"
      blurb="Stack lays children out in a column or a row, gap apart inside padding, aligned across the flow. Stacks nest; this whole screen is built from them."
    >
      <Stack direction="row" gap={12}>
        {ALIGNS.map((align) => (
          <Stack key={align} width={218} gap={6} padding={8} align={align} color={theme.surface}>
            <Label width={202} fontSize={13} color={theme.outline}>{`column, align="${align}"`}</Label>
            <Rect width={60} height={18} color={theme.primary} borderRadius={4} />
            <Rect width={140} height={18} color={theme.accent} borderRadius={4} />
            <Rect width={100} height={18} color={theme.success} borderRadius={4} />
          </Stack>
        ))}
      </Stack>
      <Stack direction="row" gap={10} padding={10} align="center" color={theme.surface}>
        <Rect width={64} height={64} color={theme.primary} borderRadius={8} />
        <Stack gap={4}>
          <Rect width={120} height={28} color={theme.accent} borderRadius={4} />
          <Rect width={120} height={28} color={theme.accent} borderRadius={4} />
        </Stack>
        <Stack direction="row" gap={4} padding={6} color={theme.surfaceAlt}>
          <Rect width={28} height={28} color={theme.success} borderRadius={14} />
          <Rect width={28} height={28} y={12} color={theme.success} borderRadius={14} />
          <Rect width={28} height={28} color={theme.success} borderRadius={14} />
        </Stack>
        <Label width={250} fontSize={13} wrap color={theme.outline}>
          A row holding a column and another row. The middle dot has y=12: across the flow, x/y offset a child.
        </Label>
      </Stack>
    </Section>
  );
}
