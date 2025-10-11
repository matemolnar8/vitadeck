import React, { useCallback, useEffect, useState } from "react";

export const App = () => {
  const [count, setCount] = useState(0);
  const doTimeout = useCallback(() => {
    setTimeout(() => {
      setCount((count) => count + 1);
      doTimeout();
    }, 1000);
  }, []);

  useEffect(() => {
    doTimeout();
  }, []);

  return (
    <vita-rect x={0} y={0} width={960} height={544} color={Colors.BLACK}>
      <vita-text color={Colors.RAYWHITE} fontSize={20} border={true}>
        Hello, VitaDeck! Updates: {count}
      </vita-text>
      <vita-rect
        x={40}
        y={40}
        width={960 - 80}
        height={544 - 80}
        color={Colors.RAYWHITE}
      >
        {Array.from({ length: 2 }).map((_, row) =>
          Array.from({ length: 4 }).map((_, col) => {
            const PAD = 20;
            const cellWidth = (960 - 80 - PAD * 3) / 4;
            const cellHeight = (544 - 80 - PAD * 1) / 2;
            const x = col * (cellWidth + PAD);
            const y = row * (cellHeight + PAD);
            const color =
              (row * 4 + col) % 2 === 0 ? Colors.LIGHTGRAY : Colors.GRAY;
            return (
              <vita-rect
                key={`${row}-${col}`}
                x={x}
                y={y}
                width={cellWidth}
                height={cellHeight}
                color={color}
              >
                <vita-text color={Colors.DARKGRAY} fontSize={20}>
                  {`(${col + 1},${row + 1})`}
                </vita-text>
              </vita-rect>
            );
          })
        )}
      </vita-rect>
    </vita-rect>
  );
};
