import assert from "node:assert/strict";
import test from "node:test";
import React, { useState, type ReactNode } from "react";
import Reconciler from "react-reconciler";
import { DefaultEventPriority, LegacyRoot } from "react-reconciler/constants";
import { dispatchPadButton, usePadButton } from "../src/pad-buttons";

const unused = () => {
  throw new Error("these tests render no host elements");
};

// A renderer for components that render null, in the legacy root mode the runtime uses: a setState outside React's
// batching renders synchronously.
const reconciler = Reconciler({
  supportsMutation: true,
  supportsPersistence: false,
  supportsHydration: false,
  isPrimaryRenderer: true,
  noTimeout: -1,
  createInstance: unused,
  createTextInstance: unused,
  appendInitialChild: unused,
  finalizeInitialChildren: () => false,
  prepareUpdate: () => null,
  shouldSetTextContent: () => false,
  getRootHostContext: () => null,
  getChildHostContext: (parent) => parent,
  getPublicInstance: unused,
  prepareForCommit: () => null,
  resetAfterCommit: () => {},
  preparePortalMount: () => {},
  clearContainer: () => {},
  scheduleTimeout: setTimeout,
  cancelTimeout: clearTimeout,
  getCurrentEventPriority: () => DefaultEventPriority,
  getInstanceFromNode: () => null,
  beforeActiveInstanceBlur: () => {},
  afterActiveInstanceBlur: () => {},
  prepareScopeUpdate: () => {},
  getInstanceFromScope: () => null,
  detachDeletedInstance: () => {},
});

function render(element: ReactNode): () => void {
  const root: unknown = reconciler.createContainer({}, LegacyRoot, null, false, null, "", () => {}, null);
  reconciler.updateContainer(element, root, null, null);
  reconciler.flushPassiveEffects();
  return () => {
    reconciler.updateContainer(null, root, null, null);
    reconciler.flushPassiveEffects();
  };
}

void test("a handler that re-renders runs once per press", () => {
  let presses = 0;

  // Like the minesweeper reset: the second setState flushes the first render's effects, so usePadButton subscribes
  // again in the middle of the dispatch.
  function Counter() {
    const [count, setCount] = useState(0);
    const [, setLabel] = useState("");
    usePadButton("triangle", {
      onPressStart: () => {
        presses++;
        if (presses > 10) throw new Error("the press is dispatched again and again");
        setCount(count + 1);
        setLabel(String(count + 1));
      },
    });
    return null;
  }

  const unmount = render(<Counter />);
  dispatchPadButton("triangle", "pressstart");
  assert.equal(presses, 1);
  dispatchPadButton("triangle", "pressstart");
  assert.equal(presses, 2);
  unmount();
});
