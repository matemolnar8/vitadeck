import React from "react";
import { useTypewriter } from "../hooks/useTypewriter";

type Props = {
  x: number;
  y: number;
  width: number;
  height: number;
  color: Color;
  messages: string[];
};

export const Typewriter = ({ x, y, width, height, color, messages }: Props) => {
  const { text, cursorOn } = useTypewriter(messages);

  return (
    <vita-rect x={x} y={y} width={width} height={height} color={color}>
      <vita-text color={Colors.RAYWHITE} fontSize={20}>
        {text}
        {cursorOn ? "_" : " "}
      </vita-text>
    </vita-rect>
  );
};
