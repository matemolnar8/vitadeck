export type Color = { r: number; g: number; b: number; a: number };

export type ColorName =
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

export type ColorsMap = { [K in ColorName]: Color };
