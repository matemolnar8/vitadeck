import { storage, TextInput, useTheme } from "@vitadeck/sdk";
import React, { useState } from "react";
import { Label, Section } from "../ui";
import { NAME_KEY } from "./StorageSection";

export function TextInputSection() {
  const { theme } = useTheme();
  const [name, setName] = useState(() => storage.getItem(NAME_KEY) ?? "");

  const confirm = (next: string) => {
    setName(next);
    storage.setItem(NAME_KEY, next);
  };

  return (
    <Section
      title="Text Input"
      blurb="A controlled field. Pressing it opens the Vita keyboard dialog; on the desktop, type in place, Enter confirms, Escape cancels. onChange fires on confirm."
    >
      <TextInput
        width={420}
        height={52}
        value={name}
        onChange={confirm}
        placeholder="Your name"
        maxLength={32}
        fontSize={22}
        color={theme.surface}
        textColor={theme.text}
        focusColor={theme.accent}
        borderRadius={8}
        autoFocus
      />
      <Label fontSize={20}>{name ? `Hello, ${name}!` : "No name confirmed yet."}</Label>
      <Label fontSize={14} wrap color={theme.outline}>
        {`Each confirmed value is saved with storage.setItem("${NAME_KEY}"), so it is still here after a Deck App Runtime Restart. The Storage section can clear it.`}
      </Label>
    </Section>
  );
}
