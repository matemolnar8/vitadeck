import { Button, Stack, storage, useTheme } from "@vitadeck/sdk";
import React, { useState } from "react";
import { Label, Section } from "../ui";

export const COUNTER_KEY = "demo.counter";
export const NAME_KEY = "demo.name";

export function StorageSection() {
  const { theme } = useTheme();
  const [counter, setCounter] = useState(() => Number(storage.getItem(COUNTER_KEY) ?? 0));
  const [name, setName] = useState(() => storage.getItem(NAME_KEY));

  const increment = () => {
    const next = counter + 1;
    setCounter(next);
    storage.setItem(COUNTER_KEY, String(next));
  };

  const reset = () => {
    storage.removeItem(COUNTER_KEY);
    storage.removeItem(NAME_KEY);
    setCounter(0);
    setName(null);
  };

  return (
    <Section
      title="Storage"
      blurb="storage keeps string pairs on disk for this Deck App (1 MiB). They survive a Deck App Runtime Restart and reinstalls; removing the Deck App deletes them."
    >
      <Stack direction="row" gap={12} align="center">
        <Button
          width={160}
          height={48}
          label="+1"
          fontSize={24}
          color={theme.buttonBackground}
          textColor={theme.buttonText}
          borderRadius={8}
          autoFocus
          onPress={increment}
        />
        <Button
          width={160}
          height={48}
          label="Reset"
          color={theme.danger}
          textColor={theme.buttonText}
          borderRadius={8}
          onPress={reset}
        />
      </Stack>
      <Label fontSize={20}>{`${COUNTER_KEY} = ${counter}`}</Label>
      <Label fontSize={20}>{`${NAME_KEY} = ${name === null ? "null" : JSON.stringify(name)}`}</Label>
      <Label fontSize={14} lines={2} color={theme.outline}>
        To see it persist, press +1 a few times, open the Shell with Start (F1), activate another Deck App, then
        VitaDeck Demo again and come back here.
      </Label>
    </Section>
  );
}
