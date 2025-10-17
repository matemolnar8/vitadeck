import React from "react";

type Props = {
  inset: number;
  innerW: number;
  innerH: number;
  padding: number;
  cols: number;
  rows: number;
  pulse: number;
  subtle: number;
  accent: Color;
  panel: Color;
  scanIndex: number;
};

export function PanelGrid({ inset, innerW, innerH, padding, cols, rows, pulse, subtle, accent, panel, scanIndex }: Props) {
  const cellW = (innerW - padding * (cols - 1)) / cols;
  const cellH = (innerH - padding * (rows - 1)) / rows;
  return (
    <vita-rect x={inset} y={inset} width={innerW} height={innerH} color={panel}>
      {Array.from({ length: rows }).map((_, row) =>
        Array.from({ length: cols }).map((_, col) => {
          const i = row * cols + col;
          const base = (row + col) % 2 === 0 ? Colors.LIGHTGRAY : Colors.GRAY;
          const highlight = i === scanIndex;
          const t = highlight ? 0.65 + 0.25 * pulse : 0.15 * subtle;
          const color = {
            r: (base.r + (accent.r - base.r) * t) | 0,
            g: (base.g + (accent.g - base.g) * t) | 0,
            b: (base.b + (accent.b - base.b) * t) | 0,
            a: 255,
          } as Color;
          const x = (col * (cellW + padding)) | 0;
          const y = (row * (cellH + padding)) | 0;
          return (
            <vita-rect key={`${row}-${col}`} x={x} y={y} width={cellW} height={cellH} color={color}>
              {highlight ? (
                <vita-rect x={0} y={0} width={cellW} height={cellH} variant="outline" color={{ r: 255, g: 200, b: 0, a: 255 }} />
              ) : null}
              <vita-text color={Colors.DARKGRAY} fontSize={18}>{`(${col + 1},${row + 1})`}</vita-text>
            </vita-rect>
          );
        })
      )}
    </vita-rect>
  );
}


