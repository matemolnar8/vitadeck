import React, { useState } from "react";
import { Button, Rect, Screen, Text, insetContent, useTheme } from "@vitadeck/sdk";
import { Counter } from "./Counter";

export default function HelloDeckApp() {
  const { theme } = useTheme();
  const [pressedButton, setPressedButton] = useState<string | null>(null);
  const { x: insetX, y: insetY, width: cw, height: ch } = insetContent();
  const bannerH = 80;
  const gap = 16;
  const counterY = insetY + bannerH + gap;
  const counterFootprintH = 90;
  const panelY = counterY + counterFootprintH + gap;
  const panelH = ch - (panelY - insetY);

  return (
    <Screen>
      <Rect x={insetX} y={insetY} width={cw} height={bannerH} color={theme.surface} borderRadius={0.08}>
        <Text fontSize={28} color={theme.text}>
          Hello, world!
        </Text>
      </Rect>

      <Counter x={insetX + 10} y={counterY} />

      <Rect x={insetX} y={panelY} width={cw} height={panelH} color={theme.surface} borderRadius={0.08}>
        <Text fontSize={22} color={theme.text}>
          Overlapping Buttons Test
        </Text>
        <Text fontSize={18} color={theme.text}>
          Press the overlapping area to test which button handles the press
        </Text>
        <Text fontSize={18} color={theme.text}>
          {pressedButton ? `Last pressed: ${pressedButton} button` : "No button pressed yet"}
        </Text>

        <Button
          x={50}
          y={100}
          width={300}
          height={60}
          color={theme.danger}
          textColor={theme.navText}
          label="Bottom Button"
          onPress={() => setPressedButton("bottom")}
          borderRadius={0.12}
        />

      <Button
        x={200}
        y={120}
        width={300}
        height={60}
        color={theme.success}
        textColor={theme.navText}
        label="Top Button"
        onPress={() => setPressedButton("top")}
        borderRadius={0.12}
      />
      </Rect>
    </Screen>
  );
}
