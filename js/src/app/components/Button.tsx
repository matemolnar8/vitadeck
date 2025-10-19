import React, { useState } from "react";

type Props = {
  x: number;
  y: number;
  width: number;
  height: number;
  color?: Color;
  label: string;
  onClick?: () => void;
};

export const Button = ({ x, y, width, height, color = Colors.DARKBLUE, label, onClick }: Props) => {
  const [hovered, setHovered] = useState(false);
  const [pressed, setPressed] = useState(false);

  const visualColor = pressed
    ? Colors.DARKBLUE
    : hovered
    ? Colors.SKYBLUE
    : color || Colors.DARKBLUE;

  return (
    <vita-rect
      x={x}
      y={y}
      width={width}
      height={height}
      color={visualColor}
      onClick={onClick}
      onMouseEnter={() => setHovered(true)}
      onMouseLeave={() => { setHovered(false); setPressed(false); }}
      onMouseDown={() => setPressed(true)}
      onMouseUp={() => setPressed(false)}
    >
      <vita-text color={Colors.RAYWHITE} fontSize={20}>
        {label}
      </vita-text>
    </vita-rect>
  );
};


