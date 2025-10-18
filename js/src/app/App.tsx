import React from "react";
import { Typewriter } from "./components/Typewriter";

export const App = () => {
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
      <Typewriter
        x={0}
        y={30}
        width={960}
        height={30}
        color={Colors.DARKGREEN}
        messages={[
          "Hello, VitaDeck!",
          "React renderer on PS Vita",
          "Made with raylib",
          "Enjoy!",
        ]}
      />
    </vita-rect>
  );
};
