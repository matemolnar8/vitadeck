import React, { useState } from "react";
import { useTheme } from "../theme";

type Props = {
  x: number;
  y: number;
};

export const Counter = ({ x, y }: Props) => {
  const [count, setCount] = useState(0);
  const { theme } = useTheme();

  const handleClick = () => {
    setCount((count) => count + 1);
  };

  return (
    <>
      <vita-button x={x} y={y} width={180} height={40} label={"Increment"} onClick={handleClick} />
      <vita-rect x={x} y={y + 50} width={180} height={40} color={theme.surface}>
        <vita-text fontSize={24} color={theme.text}>
          Count: {count}
        </vita-text>
      </vita-rect>
    </>
  );
};
