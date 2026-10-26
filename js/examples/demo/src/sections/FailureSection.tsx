import { Button, useTheme } from "@vitadeck/sdk";
import React from "react";
import { Label, Section } from "../ui";

export function FailureSection() {
  const { theme } = useTheme();
  return (
    <Section
      title="Deck App Failure Screen"
      blurb="An uncaught error in render, an effect, a handler or a timer, or an unhandled promise rejection, replaces the Deck App with the Deck App Failure Screen."
    >
      <Button
        width={260}
        height={52}
        label="Throw an error"
        color={theme.danger}
        textColor={theme.buttonText}
        borderRadius={8}
        onPress={() => {
          throw new Error("Thrown on purpose by the VitaDeck Demo");
        }}
      />
      <Label fontSize={16} wrap color={theme.warning}>
        To recover, press Start (F1 on the desktop) to open the VitaDeck Shell, activate another Deck App, then VitaDeck
        Demo again. That Deck App Runtime Restart clears the Failure Screen; Storage keeps its values.
      </Label>
    </Section>
  );
}
