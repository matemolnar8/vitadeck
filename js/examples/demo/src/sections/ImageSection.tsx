import { Image, Stack, useTheme } from "@vitadeck/sdk";
import React from "react";
import { Label, Section } from "../ui";

export function ImageSection() {
  const { theme } = useTheme();
  return (
    <Section
      title="Image"
      blurb='A Deck App Image is declared in vitadeck.config.json ("logo": a 96×64 PNG) and referenced by name. Give width or height to keep its aspect ratio, or both to stretch it.'
    >
      <Stack direction="row" gap={24} align="end">
        <Stack gap={6} align="center">
          <Image image="logo" height={32} />
          <Label width={110} fontSize={13} color={theme.outline} align="center">
            height=32
          </Label>
        </Stack>
        <Stack gap={6} align="center">
          <Image image="logo" height={96} />
          <Label width={150} fontSize={13} color={theme.outline} align="center">
            height=96
          </Label>
        </Stack>
        <Stack gap={6} align="center">
          <Image image="logo" width={192} />
          <Label width={192} fontSize={13} color={theme.outline} align="center">
            width=192
          </Label>
        </Stack>
        <Stack gap={6} align="center">
          <Image image="logo" width={160} height={64} />
          <Label width={160} fontSize={13} color={theme.outline} align="center">
            160×64, stretched
          </Label>
        </Stack>
      </Stack>
    </Section>
  );
}
