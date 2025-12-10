import * as React from "react";

type TextChild = string | number | boolean | null | undefined;
type JSXPropsWithKey<T> = T & { key?: string };

declare global {
  namespace JSX {
    interface IntrinsicElements {
      "vita-text": JSXPropsWithKey<{
        children: TextChild | TextChild[];
        color?: Color;
        border?: boolean;
        fontSize?: number;
      }>;
      "vita-rect": JSXPropsWithKey<{
        x: number;
        y: number;
        width: number;
        height: number;
        variant?: "fill" | "outline";
        color?: Color;
        borderColor?: Color;
        onClick?: (() => void) | undefined;
        onMouseDown?: () => void;
        onMouseUp?: () => void;
        onMouseEnter?: () => void;
        onMouseLeave?: () => void;
        children?: React.ReactNode | React.ReactNode[];
      }>;
      "vita-button": JSXPropsWithKey<{
        x: number;
        y: number;
        width: number;
        height: number;
        color?: Color;
        label: string;
        onClick?: () => void;
        onMouseDown?: () => void;
        onMouseUp?: () => void;
        onMouseEnter?: () => void;
        onMouseLeave?: () => void;
      }>;
    }
  }

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

  function getInteractiveState(id: string): {
    hovered: boolean;
    pressed: boolean;
  };

  // Native instance tree mutation functions
  function nativeCreateRect(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    hasFill: boolean,
    fillR: number,
    fillG: number,
    fillB: number,
    fillA: number,
    hasOutline: boolean,
    outlineR: number,
    outlineG: number,
    outlineB: number,
    outlineA: number,
  ): void;

  function nativeCreateText(
    id: string,
    fontSize: number,
    hasColor: boolean,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    border: boolean,
  ): void;

  function nativeCreateButton(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    label: string,
    fontSize: number,
  ): void;

  function nativeCreateRawText(id: string, text: string): void;

  function nativeAppendChild(parentId: string, childId: string): void;
  function nativeInsertBefore(parentId: string, childId: string, beforeId: string): void;
  function nativeRemoveChild(parentId: string, childId: string): void;
  function nativeDestroyInstance(id: string): void;

  function nativeUpdateRect(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    hasFill: boolean,
    fillR: number,
    fillG: number,
    fillB: number,
    fillA: number,
    hasOutline: boolean,
    outlineR: number,
    outlineG: number,
    outlineB: number,
    outlineA: number,
  ): void;

  function nativeUpdateText(
    id: string,
    fontSize: number,
    hasColor: boolean,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    border: boolean,
  ): void;

  function nativeUpdateButton(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    label: string,
    fontSize: number,
  ): void;

  function nativeUpdateRawText(id: string, text: string): void;

  function nativeClearContainer(): void;

  function setTimeout(callback: () => void, delay: number): number;
  function clearTimeout(id: number): void;
  function setInterval(callback: () => void, delay: number): number;
  function clearInterval(id: number): void;

  function getTime(): number;

  var console: {
    log: (...args: unknown[]) => void;
    info: (...args: unknown[]) => void;
    debug: (...args: unknown[]) => void;
    warn: (...args: unknown[]) => void;
    error: (...args: unknown[]) => void;
  };
}
