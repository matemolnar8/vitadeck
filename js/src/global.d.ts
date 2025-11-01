import * as React from "react";

declare global {
  type Color = { r: number; g: number; b: number; a: number };

  type ColorName =
    | "LIGHTGRAY"
    | "GRAY"
    | "DARKGRAY"
    | "YELLOW"
    | "GOLD"
    | "ORANGE"
    | "PINK"
    | "RED"
    | "MAROON"
    | "GREEN"
    | "LIME"
    | "DARKGREEN"
    | "SKYBLUE"
    | "BLUE"
    | "DARKBLUE"
    | "PURPLE"
    | "VIOLET"
    | "DARKPURPLE"
    | "BEIGE"
    | "BROWN"
    | "DARKBROWN"
    | "WHITE"
    | "BLACK"
    | "BLANK"
    | "MAGENTA"
    | "RAYWHITE";

  var Colors: { [K in ColorName]: Color };

  function drawRect(x: number, y: number, width: number, height: number, fillColor?: Color, outlineColor?: Color): void;
  function drawText(x: number, y: number, fontSize: number, text: string, color?: Color, border?: boolean): void;

  function syncInteractiveRectsToNative(): void;
  function getInteractiveState(id: string): {
    hovered: boolean;
    pressed: boolean;
  };

  function setTimeout(callback: () => void, delay: number): number;
  function clearTimeout(id: number): void;
  function setInterval(callback: () => void, delay: number): number;
  function clearInterval(id: number): void;

  var console: {
    log: (...args: unknown[]) => void;
    info: (...args: unknown[]) => void;
    debug: (...args: unknown[]) => void;
    warn: (...args: unknown[]) => void;
    error: (...args: unknown[]) => void;
  };
}
