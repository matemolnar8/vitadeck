import React, { useCallback, useState } from "react";

type Props = {
  x: number;
  y: number;
};

export const Counter = ({ x, y }: Props) => {
  const [count, setCount] = useState(0);

  const handleClick = useCallback(() => {
    setCount((count) => count + 1);
  }, []);

  return (
    <>
      <vita-button
        x={x}
        y={y}
        width={180}
        height={40}
        label={"Increment"}
        onClick={handleClick}
      />
      <vita-rect x={x} y={y + 50} width={180} height={40} color={Colors.DARKBLUE}>
        <vita-text fontSize={24} color={Colors.RAYWHITE}>
          Count: {count}
        </vita-text>
      </vita-rect>
    </>
  );
};
