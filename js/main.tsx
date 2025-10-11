import React from "react";
import { VitadeckRenderer } from "./react-renderer";
import { renderVitadeckElement } from "./renderer";
import { App } from "./app/App";

function logError(error: unknown) {
  if (!error) {
    debug("Error: unknown");
    return;
  }

  debug(
    "Error: " + (typeof error === "object")
      ? error.toString?.() || String(error)
      : String(error)
  );
  if ((error as Error).stack) {
    debug((error as Error).stack);
  }
}

export function updateContainer() {
  try {
    VitadeckRenderer.updateContainer(<App />, root);
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

const root = VitadeckRenderer.createContainer(
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
