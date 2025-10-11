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

  function drawRect(
    x: number,
    y: number,
    width: number,
    height: number,
    color?: Color
  ): void;
  function drawRectOutline(
    x: number,
    y: number,
    width: number,
    height: number,
    color?: Color
  ): void;
  function drawText(
    x: number,
    y: number,
    fontSize: number,
    text: string,
    color?: Color,
    border?: boolean
  ): void;

  function setTimeout(callback: () => void, delay: number): number;
  function clearTimeout(id: number): void;

  var console: {
    log: (...args: any[]) => void;
    info: (...args: any[]) => void;
    debug: (...args: any[]) => void;
    warn: (...args: any[]) => void;
    error: (...args: any[]) => void;
  };
}
