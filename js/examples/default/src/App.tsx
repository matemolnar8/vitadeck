import { Rect, Screen, Text, insetContent, useTheme } from "@vitadeck/sdk";
import React from "react";

export default function App() {
  const { theme } = useTheme();
  const { x, y, width, height } = insetContent();

  return (
    <Screen>
      <Rect x={x} y={y} width={width} height={height} color={theme.surface}>
        <Text fontSize={28} color={theme.text}>
          Hello from default
        </Text>
      </Rect>
    </Screen>
  );
}
