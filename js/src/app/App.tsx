import React from "react";
import { useTicker } from "../app/hooks/useTicker";
import { useTypewriter } from "../app/hooks/useTypewriter";
import { Header } from "../app/components/Header";
import { PanelGrid } from "../app/components/PanelGrid";
import { Scanline } from "../app/components/Scanline";
import { BouncingBar } from "../app/components/BouncingBar";

export const App = () => {
  // ----------------------------------------------------------------------------
  // Single ticker driving all animations
  // ----------------------------------------------------------------------------
  const { tick, beatIndex, cursorOn } = useTicker(33);
  const typed = useTypewriter(
    ["Hello, VitaDeck!", "React renderer on PS Vita", "Made with raylib", "Enjoy!"],
    tick
  );

  // Helpers
  function clampByte(v: number): number {
    if (v < 0) return 0;
    if (v > 255) return 255;
    return v | 0;
  }

  function mixColor(a: Color, b: Color, t: number): Color {
    const u = t < 0 ? 0 : t > 1 ? 1 : t;
    return {
      r: clampByte(a.r + (b.r - a.r) * u),
      g: clampByte(a.g + (b.g - a.g) * u),
      b: clampByte(a.b + (b.b - a.b) * u),
      a: clampByte(a.a + (b.a - a.a) * u),
    };
  }

  // No other intervals/timeouts – everything derives from ticker

  // Derived animation values
  const pulse = (Math.sin(tick / 20) + 1) * 0.5; // 0..1
  const subtle = (Math.sin(tick / 60) + 1) * 0.5; // 0..1
  const bgColor: Color = { r: 6, g: 10 + (subtle * 18) | 0, b: 18 + (pulse * 48) | 0, a: 255 };
  const rimColor: Color = mixColor(Colors.DARKBLUE, Colors.SKYBLUE, 0.35 + 0.35 * pulse);
  const panelColor: Color = mixColor(Colors.RAYWHITE, Colors.LIGHTGRAY, 0.15 + 0.1 * subtle);
  const accentColor: Color = mixColor(Colors.GOLD, Colors.ORANGE, 0.3 + 0.4 * pulse);
  const headerColor: Color = mixColor(Colors.DARKBLUE, Colors.BLUE, 0.45 + 0.35 * subtle);
  const headerShadowColor: Color = { r: 0, g: 0, b: 0, a: 60 };

  // Layout
  const inset = 40;
  const innerW = 960 - inset * 2;
  const innerH = 544 - inset * 2;
  const padding = 20;
  const cols = 4;
  const rows = 2;
  const cellW = (innerW - padding * (cols - 1)) / cols;
  const cellH = (innerH - padding * (rows - 1)) / rows;

  // Scanning highlight index across 8 cells
  const scanIndex = beatIndex % (cols * rows);
  const scanY = ((tick * 2) % innerH) | 0;
  const scanColor: Color = { r: accentColor.r, g: accentColor.g, b: accentColor.b, a: 70 };
  const scanColorSoft: Color = { r: accentColor.r, g: accentColor.g, b: accentColor.b, a: 30 };

  return (
    <vita-rect x={0} y={0} width={960} height={544} color={bgColor}>
      {/* Rim / vignette frame */}
      <vita-rect x={0} y={0} width={960} height={544} variant="outline" color={rimColor} />

      <Header text={typed} cursorOn={cursorOn} color={headerColor} shadow={headerShadowColor} />

      <PanelGrid
        inset={inset}
        innerW={innerW}
        innerH={innerH}
        padding={padding}
        cols={cols}
        rows={rows}
        pulse={pulse}
        subtle={subtle}
        accent={accentColor}
        panel={panelColor}
        scanIndex={scanIndex}
      />

      <Scanline x={inset} y={scanY} width={innerW} color={scanColor} soft={scanColorSoft} />

      <BouncingBar innerW={innerW} innerH={innerH} cellW={cellW} cellH={cellH} tick={tick} color={accentColor} originX={inset} originY={inset} />
    </vita-rect>
  );
};
