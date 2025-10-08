import React from "react";
import { Container, VitadeckRenderer } from "./renderer";
import { App } from "./app/App";

const TRACE = false;

function renderVitadeckElement(container: Container) {
  if (TRACE) debug("renderVitadeckElement: " + JSON.stringify(container));
  for (const child of container.children) {
    if (TRACE) debug("child: " + JSON.stringify(child));

    if (child.type === "vita-text") {
      let text = "";
      child.children.forEach((child) => {
        if (child.type === "RawText") {
          text += child.text;
        }
      });
      print(text);
    } else {
      throw "TODO child.type: " + child.type;
    }
  }
}

const root = VitadeckRenderer.createContainer(
  { children: [] },
  0,
  null,
  true,
  null,
  "vitadeck_react_",
  () => {},
  null
);


export function updateContainer() {
  try {
    VitadeckRenderer.updateContainer(
      <App />,
      root
    );
  } catch (error: unknown) {
    logError(error);
  }
}

export function render() {
  try {
    renderVitadeckElement(root.containerInfo as Container);
  } catch (error: unknown) {
    logError(error);
  }
}

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
