import { Rect, Stack, usePadButton, useTheme, type PadButton } from "@vitadeck/sdk";
import React, { useState } from "react";
import { Label, Section } from "../ui";

const INDICATORS = [
  { button: "circle", name: "Circle", hostKey: "Backspace" },
  { button: "triangle", name: "Triangle", hostKey: "T" },
  { button: "square", name: "Square", hostKey: "S" },
] as const satisfies ReadonlyArray<{ button: PadButton; name: string; hostKey: string }>;

function useHeld(button: PadButton) {
  const [held, setHeld] = useState(false);
  usePadButton(button, {
    onPressStart: () => {
      setHeld(true);
    },
    onPressEnd: () => {
      setHeld(false);
    },
  });
  return held;
}

function Indicator({ button, name, hostKey }: (typeof INDICATORS)[number]) {
  const { theme } = useTheme();
  const held = useHeld(button);
  return (
    <Stack gap={6} align="center">
      <Rect
        width={140}
        height={56}
        color={held ? theme.accent : theme.surface}
        borderColor={theme.outline}
        borderRadius={28}
      >
        <Label width={140} fontSize={18} align="center" y={16}>
          {name}
        </Label>
      </Rect>
      <Label width={140} fontSize={13} align="center" color={theme.outline}>
        {`desktop: ${hostKey}`}
      </Label>
    </Stack>
  );
}

export function PadButtonSection() {
  const { theme } = useTheme();
  const [lampOn, setLampOn] = useState(false);
  usePadButton("square", {
    onPressStart: () => {
      setLampOn((on) => !on);
    },
  });

  return (
    <Section
      title="Pad Buttons"
      blurb="usePadButton(button, { onPressStart, onPressEnd }) listens while the component is mounted. Hold one to light it up."
    >
      <Stack direction="row" gap={16}>
        {INDICATORS.map((indicator) => (
          <Indicator key={indicator.button} {...indicator} />
        ))}
      </Stack>
      <Stack direction="row" gap={12} align="center">
        <Rect
          width={32}
          height={32}
          color={lampOn ? theme.warning : theme.surface}
          borderColor={theme.outline}
          borderRadius={16}
        />
        <Label width={600}>{`Square (S) toggles this lamp: ${lampOn ? "on" : "off"}`}</Label>
      </Stack>
      <Label fontSize={14} wrap color={theme.outline}>
        L and R (keys L and R) switch sections: App.tsx listens to them with usePadButton too, so every subscriber of a
        button hears it. Start (F1) opens the VitaDeck Shell and Select is reserved; neither reaches a Deck App.
      </Label>
    </Section>
  );
}
