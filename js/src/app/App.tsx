import React, { useCallback, useState } from "react";
import { Typewriter } from "./components/Typewriter";
import { Button } from "./components/Button";

export const App = () => {
  const [count, setCount] = useState(0);

  const handleClick = useCallback(() => {
    setCount((count) => count + 1);
  }, []);

  return (
    <vita-rect x={0} y={0} width={960} height={544} color={Colors.BLACK}>
      <Typewriter
        x={0}
        y={0}
        width={960}
        height={30}
        color={Colors.BLUE}
        messages={[
          "Hello, VitaDeck!",
          "React renderer on PS Vita",
          "Made with raylib",
          "Enjoy!",
        ]}
      />
      <Button
        x={20}
        y={80}
        width={180}
        height={40}
        label={"Click Me"}
        onClick={handleClick}
      />
      <vita-rect x={20} y={120} width={180} height={40} color={Colors.DARKBLUE}>
        <vita-text fontSize={20} color={Colors.RAYWHITE}>
          Count: {count}
        </vita-text>
      </vita-rect>
      <Button
        x={20}
        y={160}
        width={180}
        height={40}
        label={"Click Me 2"}
        onClick={handleClick}
      />
    </vita-rect>
  );
};
