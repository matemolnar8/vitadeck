import { Button, Rect, Screen, Text, hostControl, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useEffect, useState } from "react";

declare global {
  // Set by native when VITADECK_E2E_HOST_ECHO=1 (headless integration tests only).
  var __vitadeckE2eHostEcho: boolean | undefined;
}

export default function App() {
  const { theme } = useTheme();
  const { x, y, width, height } = insetContent();
  const [hostStatus, setHostStatus] = useState("Host: not tested");

  async function testHostEcho() {
    setHostStatus("Host: calling echo...");
    try {
      const result = await hostControl.command("host.echo", { from: "default-deck-app" });
      if (result.ok) {
        setHostStatus(`Host: echo ok @ ${result.receivedAt}`);
      } else {
        setHostStatus(`Host: ${result.code}`);
      }
    } catch (error) {
      setHostStatus(`Host: ${error instanceof Error ? error.message : "error"}`);
    }
  }

  useEffect(() => {
    if (!globalThis.__vitadeckE2eHostEcho) return;
    const timer = setTimeout(() => {
      void testHostEcho();
    }, 4000);
    return () => clearTimeout(timer);
  }, []);

  return (
    <Screen>
      <Rect x={x} y={y} width={width} height={height} color={theme.surface}>
        <Text fontSize={28} color={theme.text}>
          Hello from default
        </Text>
        <Text fontSize={18} color={theme.text}>
          {hostStatus}
        </Text>
        <Button
          x={x + 20}
          y={y + 120}
          width={320}
          height={56}
          color={theme.primary}
          textColor={theme.navText}
          label="Host echo"
          onPress={() => {
            void testHostEcho();
          }}
          borderRadius={0.1}
        />
      </Rect>
    </Screen>
  );
}
