import React from "react";
import { Counter } from "../components/Counter";

export const Hello = () => {
  return (
    <>
      <vita-rect x={20} y={20} width={920} height={80} color={Colors.DARKPURPLE}>
        <vita-text fontSize={28} color={Colors.RAYWHITE}>
          Hello, world!
        </vita-text>
      </vita-rect>

      <Counter x={30} y={130} />
    </>
  );
};


