import React, { StrictMode } from "react";
import { App } from "./app/App";
import { ThemeProvider } from "./app/theme";
import { onInputEventFromNative } from "./input";
import { render } from "./vitadeck-react-reconciler-mutation";

export function updateContainer() {
  render(
    <StrictMode>
      <ThemeProvider>
        <App />
      </ThemeProvider>
    </StrictMode>,
  );
}

export const input = {
  onInputEventFromNative,
};

console.debug("main.tsx loaded");
