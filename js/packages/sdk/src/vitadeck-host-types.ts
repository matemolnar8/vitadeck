import type { ReactNode } from "react";
import type { Color, FontName, ImageName } from "./types";

type TextChild = string | number | boolean | null | undefined;
type WithKey<T> = T & { key?: string };

export type TextAlign = "left" | "center" | "right";
export type TextWrap = "none" | "word";

export type VitaTextProps = WithKey<{
  children: TextChild | TextChild[];
  color?: Color;
  border?: boolean;
  font?: FontName;
  fontSize?: number;
  x?: number;
  y?: number;
  width?: number;
  align?: TextAlign;
  wrap?: TextWrap;
  lineHeight?: number;
}>;

/**
 * Relative to the parent. In a `Stack` or `Scroll` the main-axis coordinate is a margin before the child and the
 * cross-axis one an offset from its aligned position.
 */
type Position = {
  /** Defaults to 0. */
  x?: number;
  /** Defaults to 0. */
  y?: number;
};

export type VitaRectProps = WithKey<
  Position & {
    width: number;
    height: number;
    variant?: "fill" | "outline";
    color?: Color;
    borderColor?: Color;
    /** Corner radius in pixels (CSS `border-radius` semantics). */
    borderRadius?: number;
    children?: ReactNode | ReactNode[];
  }
>;

export type FocusProps = {
  /** Called when this control takes **Focus** (D-pad navigation, autoFocus, or mouse hover on the desktop host). */
  onFocus?: () => void;
  /** Called when this control loses **Focus**. */
  onBlur?: () => void;
  /**
   * Shows **Focus**: a Button fills with it, defaulting to `color` lightened by 40%; a TextInput outlines with it,
   * defaulting to `textColor`.
   */
  focusColor?: Color;
  /** Take **Focus** on mount; when several mount together, the last one wins. */
  autoFocus?: boolean;
};

export type VitaButtonProps = WithKey<
  FocusProps &
    Position & {
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
    }
>;

export type VitaTextInputProps = WithKey<
  FocusProps &
    Position & {
      width: number;
      height: number;
      value: string;
      /** Called once with the final text when the user confirms an edit; cancelling calls nothing. */
      onChange: (value: string) => void;
      /** Shown dimmed while `value` is empty; also the title of the Vita's keyboard dialog. */
      placeholder?: string;
      /** Longest text, counted like `string.length`. Defaults to 256; at most 2048. */
      maxLength?: number;
      font?: FontName;
      fontSize?: number;
      color?: Color;
      textColor?: Color;
      /** Corner radius in pixels (CSS `border-radius` semantics). */
      borderRadius?: number;
    }
>;

export type VitaScrollProps = WithKey<{
  x: number;
  y: number;
  width: number;
  height: number;
  /** Optional background fill. */
  color?: Color;
  /** Vertical gap in pixels between stacked children. */
  gap?: number;
  /** Inner padding in pixels around the stacked content. */
  padding?: number;
  children?: ReactNode | ReactNode[];
}>;

export type StackDirection = "column" | "row";
export type StackAlign = "start" | "center" | "end";

export type VitaStackProps = WithKey<
  Position & {
    /** Defaults to the content width plus padding. */
    width?: number;
    /** Defaults to the content height plus padding. */
    height?: number;
    /** Axis the children flow along. Defaults to `"column"`. */
    direction?: StackDirection;
    /** Pixels between flowed children. Defaults to 0. */
    gap?: number;
    /** Pixels between the edges and the children. Defaults to 0. */
    padding?: number;
    /** Cross-axis placement of each child. Defaults to `"start"`. */
    align?: StackAlign;
    /** Optional background fill. */
    color?: Color;
    children?: ReactNode | ReactNode[];
  }
>;

export type VitaImageProps = WithKey<Position & { image: ImageName }> &
  ({ width: number; height: number } | { width: number; height?: never } | { height: number; width?: never });

export type VitaHostPropsByType = {
  "vita-text": VitaTextProps;
  "vita-rect": VitaRectProps;
  "vita-button": VitaButtonProps;
  "vita-text-input": VitaTextInputProps;
  "vita-scroll": VitaScrollProps;
  "vita-stack": VitaStackProps;
  "vita-image": VitaImageProps;
};

export type VitaHostType = keyof VitaHostPropsByType;
