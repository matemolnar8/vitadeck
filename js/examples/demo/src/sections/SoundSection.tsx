import { Button, playSound, Stack, stopSound, useTheme } from "@vitadeck/sdk";
import React, { useEffect, useState } from "react";
import { Label, Section } from "../ui";

export function SoundSection() {
  const { theme } = useTheme();
  const [looping, setLooping] = useState(false);

  useEffect(() => {
    return () => {
      stopSound("loop");
    };
  }, []);

  const toggleLoop = () => {
    if (looping) {
      stopSound("loop");
    } else {
      playSound("loop", { loop: true });
    }
    setLooping(!looping);
  };

  return (
    <Section
      title="Sound"
      blurb='A Deck App Sound is declared in vitadeck.config.json ("chime": a WAV, "loop": an MP3) and played by name. playSound restarts a sound that is already playing; different sounds mix.'
    >
      <Stack direction="row" gap={12} align="center">
        <Button
          width={200}
          height={48}
          label="Play chime"
          color={theme.buttonBackground}
          textColor={theme.buttonText}
          borderRadius={8}
          autoFocus
          onPress={() => {
            playSound("chime");
          }}
        />
        <Button
          width={200}
          height={48}
          label={looping ? "Stop loop" : "Start loop"}
          color={looping ? theme.danger : theme.buttonBackground}
          textColor={theme.buttonText}
          borderRadius={8}
          onPress={toggleLoop}
        />
      </Stack>
      <Label fontSize={20}>{looping ? "loop: playing, repeats until stopped" : "loop: stopped"}</Label>
      <Label fontSize={14} wrap color={theme.outline}>
        Sounds keep playing while you move to another section, except the loop, which this section stops when it
        unmounts. Switching Deck Apps stops every sound.
      </Label>
    </Section>
  );
}
