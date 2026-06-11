import React, { type ReactNode } from "react";
import { vitaHost } from "./host-elements";
import { useTheme, type Theme, type ThemeContextValue, type ThemeName } from "./theme";
import type { Color, ColorsMap } from "./types";
import type { VitaRectProps, VitaScrollProps, VitaTextProps } from "./vitadeck-host-types";

export const VITA_SCREEN = { width: 960, height: 544 } as const;
export const VITA_INSET = 20;

export function insetContent(): { x: number; y: number; width: number; height: number } {
  return {
    x: VITA_INSET,
    y: VITA_INSET,
    width: VITA_SCREEN.width - VITA_INSET * 2,
    height: VITA_SCREEN.height - VITA_INSET * 2,
  };
}

export type ScreenProps = {
  children?: ReactNode;
  color?: Color;
};

export function Screen({ children, color }: ScreenProps) {
  const { theme } = useTheme();
  return vitaHost(
    "vita-rect",
    { x: 0, y: 0, width: VITA_SCREEN.width, height: VITA_SCREEN.height, color: color ?? theme.background },
    children,
  );
}

export type RectProps = Omit<VitaRectProps, "key">;

export function Rect(props: RectProps) {
  return vitaHost("vita-rect", props);
}

export type TextProps = Omit<VitaTextProps, "key">;

export function Text(props: TextProps) {
  return vitaHost("vita-text", props);
}

export type ScrollProps = Omit<VitaScrollProps, "key">;

/**
 * Scrollable container with a minimal column layout: direct Rect and Button
 * children are stacked vertically (`gap` pixels apart, inside `padding`).
 * A stacked child's `y` acts as a top margin and its `x` is relative to the
 * padded content area. Content taller than the viewport can be scrolled by
 * focusing the container and using D-pad/arrow keys, the left analog stick,
 * or the mouse wheel.
 */
export function Scroll(props: ScrollProps) {
  return vitaHost("vita-scroll", props);
}

export type ButtonProps = {
  x: number;
  y: number;
  width: number;
  height: number;
  color?: Color;
  textColor?: Color;
  /** Corner radius in pixels (CSS `border-radius` semantics). */
  borderRadius?: number;
  label: string;
  onPress?: () => void;
  onPressStart?: () => void;
  onPressEnd?: () => void;
};

export function Button({ onPress, onPressStart, onPressEnd, ...props }: ButtonProps) {
  return vitaHost("vita-button", { ...props, onPress, onPressStart, onPressEnd });
}

const runtimeColors = (globalThis as unknown as { Colors: ColorsMap }).Colors;
export { runtimeColors as Colors, useTheme };
export type { Theme, ThemeContextValue, ThemeName };
export type { Color, ColorName, ColorsMap, FontName, VitaDeckFontMap } from "./types";
