import React from "react";
import { Counter } from "../components/Counter";
import { useTheme } from "../theme";

export const Hello = () => {
  const { theme } = useTheme();
  return (
    <>
      <vita-rect x={20} y={20} width={920} height={80} color={theme.surface}>
        <vita-text fontSize={28} color={theme.text}>
          Hello, world!
        </vita-text>
      </vita-rect>

      <Counter x={30} y={130} />
    </>
  );
};
