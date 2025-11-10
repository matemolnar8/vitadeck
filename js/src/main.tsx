import React, { StrictMode } from "react";
import { MainLauncher } from "./app/MainLauncher";
import { ThemeProvider } from "./app/theme";
import { commands } from "./drawlist";
import { interactiveRects, onInputEventFromNative } from "./input";
import { getActiveReconciler, initializeReconcilerManager, updateTree } from "./reconciler-manager";

function toError(e: unknown): Error {
  if (e instanceof Error) return e;
  return new Error(typeof e === "string" ? e : String(e));
}

function logError(error: unknown) {
  const err = toError(error);
  console.error("Error:", err.message);
  if (err.stack) {
    console.error(err.stack);
  }
}

initializeReconcilerManager({
  identifierPrefix: "vitadeck_react_",
  initialActiveReconciler: "mutation",
  onError: (error: unknown) => {
    logError(error);
  },
});

export function updateContainer() {
  try {
    updateTree(
      <StrictMode>
        <ThemeProvider>
          <MainLauncher />
        </ThemeProvider>
      </StrictMode>,
    );
  } catch (error: unknown) {
    logError(error);
  }
}

// JS no longer renders; C side renders from synced flat draw list

export const input = {
  interactiveRects,
  onInputEventFromNative: onInputEventFromNative,
};

export const draw = {
  commands,
};

console.debug("main.tsx loaded");
console.debug(`Reconciler: ${getActiveReconciler().id}`);
