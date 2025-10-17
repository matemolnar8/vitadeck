import React from "react";

type Props = {
  innerW: number;
  innerH: number;
  cellW: number;
  cellH: number;
  tick: number;
  color: Color;
  originX?: number;
  originY?: number;
};

export function BouncingBar({ innerW, innerH, cellW, cellH, tick, color, originX = 0, originY = 0 }: Props) {
  const bounceW = (cellW * 0.9) | 0;
  const bounceH = (cellH * 0.35) | 0;
  const bx = ((Math.sin(tick / 35) * 0.5 + 0.5) * (innerW - bounceW)) | 0;
  const by = ((Math.cos(tick / 47) * 0.5 + 0.5) * (innerH - bounceH)) | 0;
  return (
    <>
      <vita-rect x={originX + bx} y={originY + by} width={bounceW} height={bounceH} color={color} />
      <vita-rect x={originX + bx} y={originY + by} width={bounceW} height={bounceH} variant="outline" color={Colors.DARKBROWN} />
    </>
  );
}


