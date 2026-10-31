import { Button, Rect, Screen, Stack, Text, insetContent, useTheme } from "@vitadeck/sdk";
import { createHostClient } from "@vitadeck/sdk/host-control";
import React, { useState } from "react";
import { hostPlugin } from "../host/plugin";

// Calls the most recently linked computer. Pass { hostId } to pick one from linkedHosts().
const computer = createHostClient(hostPlugin);

export default function App() {
  const { theme } = useTheme();
  const { x, y, width, height } = insetContent();
  const [reply, setReply] = useState("Press Greet to call your computer.");

  const greet = async () => {
    const result = await computer.greet({ name: "Vita" });
    setReply(result.ok ? result.message : `${result.code}: ${result.message}`);
  };

  return (
    <Screen>
      <Rect x={x} y={y} width={width} height={height} color={theme.surface}>
        <Stack x={8} y={8} gap={16}>
          <Text fontSize={28} color={theme.text}>
            Hello from vitadeck-scaffold-template
          </Text>
          <Button
            width={160}
            height={44}
            label="Greet"
            autoFocus
            onPress={() => void greet()}
            color={theme.buttonBackground}
            textColor={theme.buttonText}
          />
          <Text fontSize={20} color={theme.text} width={width - 16} wrap="word">
            {reply}
          </Text>
        </Stack>
      </Rect>
    </Screen>
  );
}
