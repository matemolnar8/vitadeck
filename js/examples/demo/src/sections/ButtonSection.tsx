import { Button, Stack, useTheme } from "@vitadeck/sdk";
import React, { useState } from "react";
import { Label, Section } from "../ui";

const IDLE_HINT = "Move Focus onto a Button to read about it here.";

export function ButtonSection() {
  const { theme } = useTheme();
  const [presses, setPresses] = useState(0);
  const [held, setHeld] = useState(false);
  const [hint, setHint] = useState(IDLE_HINT);

  const focusHint = (text: string) => ({
    onFocus: () => {
      setHint(text);
    },
    onBlur: () => {
      setHint((current) => (current === text ? IDLE_HINT : current));
    },
  });

  return (
    <Section
      title="Button"
      blurb="D-pad or arrow keys move Focus, Cross or Enter presses. The mouse focuses on hover and presses on click. The first Button below has autoFocus."
    >
      <Stack direction="row" gap={12}>
        <Button
          width={200}
          height={48}
          label={`Pressed ${presses}×`}
          color={theme.buttonBackground}
          textColor={theme.buttonText}
          borderRadius={8}
          autoFocus
          onPress={() => {
            setPresses((count) => count + 1);
          }}
          {...focusHint("onPress counts presses. autoFocus took Focus when this section mounted.")}
        />
        <Button
          width={200}
          height={48}
          label={held ? "Held…" : "Hold me"}
          color={held ? theme.success : theme.buttonBackground}
          textColor={theme.buttonText}
          borderRadius={8}
          onPressStart={() => {
            setHeld(true);
          }}
          onPressEnd={() => {
            setHeld(false);
          }}
          {...focusHint("onPressStart and onPressEnd fire when Cross, Enter or the mouse goes down and up.")}
        />
        <Button
          width={200}
          height={48}
          label="focusColor"
          color={theme.surface}
          focusColor={theme.accent}
          textColor={theme.buttonText}
          borderRadius={24}
          {...focusHint("focusColor fills the Button while it has Focus; without it, color is lightened.")}
        />
      </Stack>
      <Stack direction="row" gap={12}>
        <Button
          width={370}
          height={56}
          label="font=mono fontSize=26"
          font="mono"
          fontSize={26}
          color={theme.primary}
          textColor={theme.warning}
          {...focusHint("Buttons take a declared Deck App Font and any fontSize (default 20).")}
        />
        <Button
          width={290}
          height={56}
          label="Emoji label 👋"
          color={theme.primary}
          textColor={theme.buttonText}
          borderRadius={4}
          {...focusHint("Labels are UTF-8 text, emoji included.")}
        />
      </Stack>
      <Label fontSize={16} color={theme.warning} wrap>
        {hint}
      </Label>
    </Section>
  );
}
