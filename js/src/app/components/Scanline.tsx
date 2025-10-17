import React from "react";

type Props = { x: number; y: number; width: number; color: Color; soft: Color };

export function Scanline({ x, y, width, color, soft }: Props) {
  return (
    <>
      <vita-rect x={x} y={y - 1} width={width} height={1} color={soft} />
      <vita-rect x={x} y={y} width={width} height={5} color={color} />
      <vita-rect x={x} y={y + 5} width={width} height={1} color={soft} />
    </>
  );
}


