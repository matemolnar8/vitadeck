import * as vitadeckSdk from "@vitadeck/sdk";
import { ThemeProvider } from "@vitadeck/sdk/internal";
import React, { Component, StrictMode, type ComponentType, type ErrorInfo, type ReactNode } from "react";
import * as reactCompilerRuntime from "react-compiler-runtime";
import { installFetch } from "./fetch";
import { onInputEventFromNative, onPadButtonFromNative, onTextInputFromNative } from "./input";
import { render } from "./vitadeck-react-reconciler-mutation";

let deckAppComponent: ComponentType | null = null;
let packageLoaded = false;

globalThis.React = React;
globalThis.reactCompilerRuntime = reactCompilerRuntime;
globalThis.vitadeckSdk = vitadeckSdk;
installFetch();
globalThis.vitadeckPackage = {
  register(component) {
    deckAppComponent = component;
  },
};

function loadDeckApp(): ComponentType {
  if (deckAppComponent) return deckAppComponent;
  if (packageLoaded) throw new Error("Deck App package did not register a component.");

  packageLoaded = true;
  nativeEvalActiveDeckAppEntry();
  if (!deckAppComponent) throw new Error("Deck App package did not register a component.");
  return deckAppComponent;
}

class DeckAppErrorBoundary extends Component<{ children: ReactNode }, { failed: boolean }> {
  override state = { failed: false };

  static getDerivedStateFromError() {
    return { failed: true };
  }

  override componentDidCatch(error: unknown, info: ErrorInfo) {
    const stack = (error instanceof Error ? error.stack : undefined) ?? info.componentStack ?? "";
    nativeReportDeckAppError(String(error), stack);
  }

  override render() {
    return this.state.failed ? null : this.props.children;
  }
}

export function updateContainer() {
  const DeckApp = loadDeckApp();
  render(
    <StrictMode>
      <DeckAppErrorBoundary>
        <ThemeProvider>
          <DeckApp />
        </ThemeProvider>
      </DeckAppErrorBoundary>
    </StrictMode>,
  );
}

export const input = {
  onInputEventFromNative,
  onPadButtonFromNative,
  onTextInputFromNative,
};

console.debug("runtime.tsx loaded");
