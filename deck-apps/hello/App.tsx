import React, { useState } from "react";
import { Button, Rect, Screen, Text, useTheme } from "@vitadeck/runtime";
import { Counter } from "../shared/Counter";

export default function HelloDeckApp() {
  const { theme } = useTheme();
  const [pressedButton, setPressedButton] = useState<string | null>(null);

  return (
    <Screen>
      <Rect x={20} y={20} width={920} height={80} color={theme.surface} borderRadius={0.15}>
        <Text fontSize={28} color={theme.text}>
          Hello, world!
        </Text>
      </Rect>

      <Counter x={30} y={130} />

      <Rect x={20} y={240} width={920} height={200} color={theme.surface} borderRadius={0.15}>
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
          borderRadius={0.2}
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
          borderRadius={0.2}
        />
      </Rect>
    </Screen>
  );
}
