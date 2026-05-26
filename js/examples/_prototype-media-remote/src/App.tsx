import { Button, Rect, Screen, Text, createHostControlClient, insetContent, useTheme } from "@vitadeck/sdk";
import React, { useEffect, useState } from "react";

const client = createHostControlClient();

type Pad = {
  label: string;
  color?: "primary" | "accent" | "surface";
  run: () => ReturnType<typeof client.command>;
};

const PADS: Pad[] = [
  { label: "Prev", color: "surface", run: () => client.command("host.media.previous") },
  { label: "Play", color: "primary", run: () => client.command("host.media.play_pause") },
  { label: "Next", color: "surface", run: () => client.command("host.media.next") },
  { label: "Vol-", color: "surface", run: () => client.command("host.media.volume_down") },
  { label: "Mute", color: "accent", run: () => client.command("host.media.mute") },
  { label: "Vol+", color: "surface", run: () => client.command("host.media.volume_up") },
  { label: "Sleep", color: "surface", run: () => client.command("host.power.sleep_displays") },
  { label: "Caps", color: "surface", run: () => client.command("host.capabilities") },
];

const COLS = 3;
const PAD = 88;
const GAP = 12;

export default function App() {
  const { theme } = useTheme();
  const { x, y, width } = insetContent();
  const [state, setState] = useState({
    status: "PROTOTYPE — configure Host Control in Shell.",
    lastLabel: "",
    hostName: "",
    commands: [] as string[],
  });

  useEffect(() => {
    void (async () => {
      try {
        const caps = await client.command("host.capabilities");
        if (!caps.ok) {
          setState((s) => ({ ...s, status: `Capabilities failed: ${caps.message}` }));
          return;
        }
        setState({
          status: "Ready — tap a pad.",
          lastLabel: "",
          hostName: caps.hostName,
          commands: [...caps.commands],
        });
      } catch (error) {
        setState((s) => ({
          ...s,
          status: error instanceof Error ? error.message : "Could not reach companion.",
        }));
      }
    })();
  }, []);

  async function sendPad(pad: Pad) {
    setState((s) => ({ ...s, status: `${pad.label}: sending...`, lastLabel: pad.label }));
    try {
      const result = await pad.run();
      setState((s) => ({
        ...s,
        status: result.ok ? `${pad.label}: ok` : `${pad.label}: ${result.message}`,
        lastLabel: pad.label,
      }));
    } catch (error) {
      setState((s) => ({
        ...s,
        status: error instanceof Error ? error.message : `${pad.label}: failed`,
        lastLabel: pad.label,
      }));
    }
  }

  function padColor(kind: Pad["color"]) {
    if (kind === "primary") return theme.primary;
    if (kind === "accent") return theme.accent;
    return theme.surface;
  }

  const gridWidth = COLS * PAD + (COLS - 1) * GAP;
  const gridX = x + Math.max(0, (width - gridWidth) / 2);

  return (
    <Screen>
      <Rect x={x} y={y} width={width} height={36} color={theme.surface}>
        <Text fontSize={22} color={theme.text}>
          PROTOTYPE Media Remote
        </Text>
      </Rect>
      {PADS.map((pad, index) => {
        const col = index % COLS;
        const row = Math.floor(index / COLS);
        return (
          <Button
            key={pad.label}
            x={gridX + col * (PAD + GAP)}
            y={y + 44 + row * (PAD + GAP)}
            width={PAD}
            height={PAD}
            borderRadius={10}
            color={padColor(pad.color)}
            textColor={pad.color === "primary" ? theme.background : theme.text}
            label={pad.label}
            onPress={() => void sendPad(pad)}
          />
        );
      })}
      <Rect x={x} y={y + 44 + 3 * (PAD + GAP) + 8} width={width} height={200} color={theme.surface}>
        <Text fontSize={14} color={theme.text}>
          {`status: ${state.status}\nhost: ${state.hostName || "(unknown)"}\nlast pad: ${state.lastLabel || "(none)"}\ncommands: ${state.commands.join(", ") || "(loading)"}`}
        </Text>
      </Rect>
    </Screen>
  );
}
