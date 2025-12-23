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
      <vita-button x={x} y={y} width={180} height={40} label={"Increment"} onClick={handleClick} color={theme.primary} textColor={theme.navText} borderRadius={0.2} />
      <vita-rect x={x} y={y + 50} width={180} height={40} color={theme.surface} borderRadius={0.15}>
        <vita-text fontSize={24} color={theme.text}>
          Count: {count}
        </vita-text>
      </vita-rect>
    </>
  );
};
