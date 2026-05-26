import { Button, Rect, Screen, Text, createHostControlClient, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useMemo, useState } from "react";

const client = createHostControlClient();

export default function App() {
  const { theme } = useTheme();
  const { x, y } = insetContent();
  const [status, setStatus] = useState("Set Host Control in Shell, then press Capabilities.");

  const actions = useMemo(
    () => [
      { label: "Capabilities", run: () => client.command("host.capabilities") },
      {
        label: "Echo",
        run: () => client.command("host.echo", { source: "host-control-example", time: getTime() }),
      },
      { label: "Sleep Displays", run: () => client.command("host.power.sleep_displays") },
    ],
    [],
  );

  async function run(label: string, action: () => Promise<unknown>) {
    setStatus(`${label}: sending...`);
    try {
      const result = await action();
      setStatus(`${label}: ${JSON.stringify(result)}`);
    } catch (error) {
      setStatus(`${label}: ${error instanceof Error ? error.message : "failed"}`);
    }
  }

  return (
    <Screen>
      <Rect x={x} y={y} width={520} height={38} color={theme.surface}>
        <Text fontSize={24} color={theme.text}>
          Host Control Companion
        </Text>
      </Rect>
      {actions.map((action, index) => (
        <Button
          key={action.label}
          x={x}
          y={y + 48 + index * 52}
          width={260}
          height={42}
          color={theme.primary}
          textColor={theme.background}
          label={action.label}
          onPress={() => void run(action.label, action.run)}
        />
      ))}
      <Rect x={x + 290} y={y + 48} width={600} height={300} color={theme.surface}>
        <Text fontSize={16} color={theme.text}>
          {status}
        </Text>
      </Rect>
    </Screen>
  );
}
