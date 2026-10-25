import React, { createContext, useContext } from "react";
import type { Color } from "./types";

function hex(value: string): Color {
  const normalized = value.replace("#", "");
  return {
    r: parseInt(normalized.slice(0, 2), 16),
    g: parseInt(normalized.slice(2, 4), 16),
    b: parseInt(normalized.slice(4, 6), 16),
    a: 255,
  };
}

export type Theme = {
  background: Color;
  navBackground: Color;
  navText: Color;
  surface: Color;
  surfaceAlt: Color;
  primary: Color;
  text: Color;
  outline: Color;
  buttonBackground: Color;
  buttonText: Color;
  accent: Color;
  success: Color;
  warning: Color;
  danger: Color;
};

const theme: Theme = {
  background: hex("#000000"),
  navBackground: hex("#080b0e"),
  navText: hex("#fafafa"),
  surface: hex("#080b0e"),
  surfaceAlt: hex("#11171d"),
  primary: hex("#153847"),
  text: hex("#fafafa"),
  outline: hex("#6f8490"),
  buttonBackground: hex("#153847"),
  buttonText: hex("#fafafa"),
  accent: hex("#f7733b"),
  success: hex("#2bca75"),
  warning: hex("#eeda2b"),
  danger: hex("#d96226"),
};

export type ThemeContextValue = {
  theme: Theme;
};

const themeContextValue: ThemeContextValue = { theme };

const ThemeContext = createContext<ThemeContextValue | null>(null);

export const ThemeProvider = ({ children }: { children: React.ReactNode }) => (
  <ThemeContext.Provider value={themeContextValue}>{children}</ThemeContext.Provider>
);

export function useTheme(): ThemeContextValue {
  const ctx = useContext(ThemeContext);
  if (!ctx) throw new Error("useTheme must be used within VitaDeck runtime bootstrap");
  return ctx;
}
