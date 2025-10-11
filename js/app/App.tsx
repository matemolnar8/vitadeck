import React, { useCallback, useEffect, useState } from "react";

export const App = () => {
  const [count, setCount] = useState(0);
  const [showBox, setShowBox] = useState(false);

  const doTimeout = useCallback(() => {
    setTimeout(() => {
      setCount((count) => count + 1);
      doTimeout();
    }, 1000);
  }, []);

  useEffect(() => {
    if(count === 3) {
      setShowBox(true);
    }

    if(count === 6) {
      setShowBox(false);
    }
  }, [count]);

  useEffect(() => {
    doTimeout();
  }, []);

  return (
    <vita-rect x={0} y={0} width={960} height={544} color={Colors.RAYWHITE}>
      <vita-text color={Colors.BLACK}>Hello, VitaDeck! Updates: {count}</vita-text>
      <vita-rect x={50} y={60} width={420} height={160} color={Colors.LIGHTGRAY}>
        <vita-text color={Colors.DARKGRAY}>Boxed line 1</vita-text>
        <vita-text color={Colors.BLUE}>Counter: {count}</vita-text>
      </vita-rect>
      <vita-rect x={500} y={60} width={300} height={120} variant="outline" color={Colors.RED}>
        <vita-text color={Colors.MAROON}>Outline rect</vita-text>
      </vita-rect>
      {showBox && (
        <vita-rect x={500} y={260} width={300} height={120} variant="fill" color={Colors.RED}>
          <vita-text color={Colors.RAYWHITE}>Fill rect</vita-text>
        </vita-rect>
      )}
    </vita-rect>
  );
};
