import React from "react";

type Props = {
  text: string;
  cursorOn: boolean;
  color: Color;
  shadow: Color;
};

export function Header({ text, cursorOn, color, shadow }: Props) {
  return (
    <>
      <vita-rect x={0} y={0} width={960} height={30} color={color} />
      <vita-text color={Colors.RAYWHITE} fontSize={20}>
        {text}
        {cursorOn ? "_" : " "}
      </vita-text>
      <vita-rect x={0} y={30} width={960} height={2} color={shadow} />
    </>
  );
}


