import React, { StrictMode } from "react";
import { App } from "./app/App";
import { ThemeProvider } from "./app/theme";
import { commands } from "./drawlist";
import { interactiveRects, onInputEventFromNative } from "./input";
import { VitadeckReactReconciler as VitadeckPersistentReconciler } from "./vitadeck-react-reconciler";
import { VitadeckReactMutationReconciler } from "./vitadeck-react-reconciler-mutation";

const USE_MUTATION_RECONCILER = true;

const ActiveReconciler = USE_MUTATION_RECONCILER ? VitadeckReactMutationReconciler : VitadeckPersistentReconciler;

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

export function updateContainer() {
  try {
    ActiveReconciler.updateContainer(
      <StrictMode>
        <ThemeProvider>
          <App />
        </ThemeProvider>
      </StrictMode>,
      root,
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

const root = ActiveReconciler.createContainer(
  { children: [] },
  0,
  null,
  true,
  null,
  "vitadeck_react_",
  (error: unknown) => {
    logError(error);
  },
  null,
);

console.debug("main.tsx loaded");
console.debug(`Reconciler: ${USE_MUTATION_RECONCILER ? "mutation" : "persistent"}`);
