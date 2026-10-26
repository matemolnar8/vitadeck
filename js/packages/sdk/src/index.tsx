import React, { type ReactNode } from "react";
import { vitaHost } from "./host-elements";
import { useTheme, type Theme, type ThemeContextValue } from "./theme";
import type { Color, ColorsMap } from "./types";
import type {
  VitaButtonProps,
  VitaImageProps,
  VitaRectProps,
  VitaScrollProps,
  VitaStackProps,
  VitaTextInputProps,
  VitaTextProps,
} from "./vitadeck-host-types";

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
  const { theme } = useTheme();
  return vitaHost("vita-text", { ...props, color: props.color ?? theme.text });
}

export type StackProps = Omit<VitaStackProps, "key">;

/**
 * Lays out its Rect, Button, TextInput, Image and Stack children one after another along `direction`, `gap` pixels
 * apart inside `padding`, and places each across the other axis by `align`. A child's own `x`/`y` is a margin before
 * it along the flow and an offset across it. Text and Scroll children are not laid out; they keep their own `x`/`y`
 * inside the padding. Without `width` or `height`, the Stack fits its content.
 */
export function Stack(props: StackProps) {
  return vitaHost("vita-stack", props);
}

export type ScrollProps = Omit<VitaScrollProps, "key">;

/**
 * A column `Stack` with a fixed viewport whose content scrolls: touch drag or the left analog stick on PSVita, the
 * mouse wheel on desktop.
 */
export function Scroll(props: ScrollProps) {
  return vitaHost("vita-scroll", props);
}

// Distributes over the width/height union, which a plain `Omit` would flatten.
type OmitKey<T> = T extends unknown ? Omit<T, "key"> : never;

export type ImageProps = OmitKey<VitaImageProps>;

/**
 * Draws a declared **Deck App Image**. Provide exactly one of `width` or
 * `height` to preserve aspect ratio, or both to stretch.
 */
export function Image(props: ImageProps) {
  return vitaHost("vita-image", props);
}

export type ButtonProps = Omit<VitaButtonProps, "key">;

export function Button(props: ButtonProps) {
  return vitaHost("vita-button", props);
}

export type TextInputProps = Omit<VitaTextInputProps, "key">;

/**
 * Single-line text field. It takes **Focus** like a Button, and a Press starts an edit: the Vita opens its keyboard
 * dialog, the desktop host edits in place (type, Backspace, Enter confirms, Escape cancels). `onChange` fires once
 * when the edit is confirmed.
 */
export function TextInput(props: TextInputProps) {
  return vitaHost("vita-text-input", props);
}

export { usePadButton } from "./pad-buttons";
export type { PadButton, PadButtonHandlers, PadButtonPhase } from "./pad-buttons";
export type { FocusProps, StackAlign, StackDirection, TextAlign, TextWrap } from "./vitadeck-host-types";

const runtimeColors = (globalThis as unknown as { Colors: ColorsMap }).Colors;
export { runtimeColors as Colors, useTheme };
export type { Theme, ThemeContextValue };
export type { Color, ColorName, ColorsMap, FontName, ImageName, VitaDeckFontMap, VitaDeckImageMap } from "./types";
export { storage } from "./storage";
