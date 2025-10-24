import React from "react";
import { VitadeckReactReconciler } from "./vitadeck-react-reconciler";
import { renderVitadeckElement } from "./raylib-renderer";
import { App } from "./app/App";
import { interactiveRects, onInputEventFromNative } from "./input";

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
    VitadeckReactReconciler.updateContainer(<App />, root);
  } catch (error: unknown) {
    logError(error);
  }
}

export function render() {
  try {
    renderVitadeckElement(root.containerInfo.children);
  } catch (error: unknown) {
    logError(error);
  }
}

export const input = {
  interactiveRects,
  onInputEventFromNative: onInputEventFromNative,
};

const root = VitadeckReactReconciler.createContainer(
  { children: [] },
  0,
  null,
  true,
  null,
  "vitadeck_react_",
  (error: unknown) => {
    logError(error);
  },
  null
);

console.debug("main.tsx loaded");
