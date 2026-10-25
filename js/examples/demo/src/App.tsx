import { Button, Rect, Screen, Stack, usePadButton, useTheme } from "@vitadeck/sdk";
import React, { useState } from "react";
import { ButtonSection } from "./sections/ButtonSection";
import { FailureSection } from "./sections/FailureSection";
import { FetchSection } from "./sections/FetchSection";
import { ImageSection } from "./sections/ImageSection";
import { PadButtonSection } from "./sections/PadButtonSection";
import { RectSection } from "./sections/RectSection";
import { ScrollSection } from "./sections/ScrollSection";
import { StackSection } from "./sections/StackSection";
import { StorageSection } from "./sections/StorageSection";
import { TextInputSection } from "./sections/TextInputSection";
import { TextSection } from "./sections/TextSection";
import { TimersSection } from "./sections/TimersSection";
import { Label } from "./ui";

const SECTIONS = [
  { name: "Rect & theme", Content: RectSection },
  { name: "Text", Content: TextSection },
  { name: "Image", Content: ImageSection },
  { name: "Button", Content: ButtonSection },
  { name: "Pad Buttons", Content: PadButtonSection },
  { name: "Stack", Content: StackSection },
  { name: "Scroll", Content: ScrollSection },
  { name: "Text Input", Content: TextInputSection },
  { name: "Storage", Content: StorageSection },
  { name: "Timers", Content: TimersSection },
  { name: "fetch", Content: FetchSection },
  { name: "Failure Screen", Content: FailureSection },
] as const;

const NAV_WIDTH = 196;
const CONTENT_HEIGHT = 420;

export default function DemoDeckApp() {
  const { theme } = useTheme();
  const [current, setCurrent] = useState(0);
  const { name, Content } = SECTIONS[current] ?? SECTIONS[0];

  const step = (delta: number) => {
    setCurrent((index) => (index + delta + SECTIONS.length) % SECTIONS.length);
  };
  usePadButton("l", {
    onPressStart: () => {
      step(-1);
    },
  });
  usePadButton("r", {
    onPressStart: () => {
      step(1);
    },
  });

  return (
    <Screen>
      <Stack x={20} y={14} gap={10}>
        <Stack direction="row" width={920} height={44} padding={10} align="center" color={theme.surface}>
          <Label width={300} fontSize={22}>
            VitaDeck Demo
          </Label>
          <Label width={600} fontSize={16} align="right" color={theme.outline}>
            {`L ◀  ${current + 1}/${SECTIONS.length} · ${name}  ▶ R`}
          </Label>
        </Stack>
        <Stack direction="row" gap={12}>
          <Stack width={NAV_WIDTH} gap={4}>
            {SECTIONS.map((section, index) => (
              <Button
                key={section.name}
                width={NAV_WIDTH}
                height={31}
                label={section.name}
                fontSize={16}
                color={index === current ? theme.accent : theme.buttonBackground}
                focusColor={index === current ? theme.accent : theme.outline}
                textColor={theme.buttonText}
                borderRadius={6}
                autoFocus={index === 0}
                onPress={() => {
                  setCurrent(index);
                }}
              />
            ))}
          </Stack>
          <Rect width={920 - NAV_WIDTH - 12} height={CONTENT_HEIGHT} color={theme.surfaceAlt} borderRadius={8}>
            <Content />
          </Rect>
        </Stack>
        <Label width={920} fontSize={13} color={theme.outline} align="center">
          D-pad / arrows: move Focus · Cross / Enter: press · L / R: section · Start / F1: VitaDeck Shell
        </Label>
      </Stack>
    </Screen>
  );
}
