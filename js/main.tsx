import React from "react";
import { VitadeckReactReconciler } from "./vitadeck-react-reconciler";
import { renderVitadeckElement } from "./raylib-renderer";
import { App } from "./app/App";

function logError(error: unknown) {
  if (!error) {
    console.error("Error: unknown");
    return;
  }

  console.error(
    "Error: " + (typeof error === "object")
      ? error.toString?.() || String(error)
      : String(error)
  );
  if ((error as Error).stack) {
    console.error((error as Error).stack);
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