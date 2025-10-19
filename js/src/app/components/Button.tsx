import React, { useState } from "react";

function mixColor(color: Color, mixWith: Color, amount: number): Color {
  return {
    r: Math.round(color.r + (mixWith.r - color.r) * amount),
    g: Math.round(color.g + (mixWith.g - color.g) * amount),
    b: Math.round(color.b + (mixWith.b - color.b) * amount),
    a: color.a,
  };
}

type Props = {
  x: number;
  y: number;
  width: number;
  height: number;
  color?: Color;
  label: string;
  onClick?: () => void;
};

export const Button = ({
  x,
  y,
  width,
  height,
  color = Colors.DARKBLUE,
  label,
  onClick,
}: Props) => {
  const [hovered, setHovered] = useState(false);
  const [pressed, setPressed] = useState(false);

  const idleColor = color || Colors.DARKBLUE;
  const hoveredColor = mixColor(idleColor, Colors.WHITE, 0.4);
  const pressedColor = mixColor(idleColor, Colors.BLACK, 0.5);

  const visualColor = pressed
    ? pressedColor
    : hovered
    ? hoveredColor
    : idleColor;

  return (
    <vita-rect
      x={x}
      y={y}
      width={width}
      height={height}
      color={visualColor}
      onClick={onClick}
      onMouseEnter={() => setHovered(true)}
      onMouseLeave={() => {
        setHovered(false);
        setPressed(false);
      }}
      onMouseDown={() => setPressed(true)}
      onMouseUp={() => setPressed(false)}
    >
      <vita-text color={Colors.RAYWHITE} fontSize={20}>
        {label}
      </vita-text>
    </vita-rect>
  );
};
