import React, { useState } from "react";
import { Button, Rect, Text, useTheme } from "@vitadeck/runtime";

type Props = {
  x: number;
  y: number;
};

export const Counter = ({ x, y }: Props) => {
  const [count, setCount] = useState(0);
  const { theme } = useTheme();

  return (
    <>
      <Button
        x={x}
        y={y}
        width={180}
        height={40}
        label="Increment"
        onPress={() => setCount((prev) => prev + 1)}
        color={theme.primary}
        textColor={theme.navText}
        borderRadius={0.12}
      />
      <Rect x={x} y={y + 50} width={180} height={40} color={theme.surface} borderRadius={0.08}>
        <Text fontSize={24} color={theme.text}>
          Count: {count}
        </Text>
      </Rect>
    </>
  );
};
