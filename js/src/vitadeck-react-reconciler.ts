import Reconciler, { type HostConfig } from "react-reconciler";
import { buildAndSyncDrawState } from "./drawlist";
import { instrumentHostConfig, ReconcilerMetrics } from "./reconciler-metrics";
import { exhaustiveGuard } from "./utils";
import {
  type ChildSet,
  generateInstanceId,
  type HostContext,
  type Instance,
  type Props,
  type PropsByType,
  type TextInstance,
  type Type,
  type UpdatePayload,
  type VitaButtonInstance,
  type VitadeckContainer,
  type VitadeckPublicInstance,
  type VitaRectInstance,
  type VitaTextInstance,
} from "./vitadeck-reconciler-shared";

type SuspenseInstance = never;
type HydratableInstance = never;
type TimeoutHandle = number;
type NoTimeout = -1;

type VitadeckHostConfig = HostConfig<
  Type,
  Props,
  VitadeckContainer,
  Instance,
  TextInstance,
  SuspenseInstance,
  HydratableInstance,
  VitadeckPublicInstance,
  HostContext,
  UpdatePayload,
  ChildSet,
  TimeoutHandle,
  NoTimeout
>;

// =============================================================================
// Implementation
// Based on Tsoding's Murayact: https://github.com/tsoding/Murayact/
// =============================================================================

const TRACE = false;
const logReconcilerFunction = (name: string, ...args: unknown[]) => {
  if (TRACE) console.log(`[Reconciler]: ${name}`, ...args);
};

// React reconciler host configuration
const rawHostConfig = {
  noTimeout: -1,
  isPrimaryRenderer: true,
  supportsMutation: false,
  supportsPersistence: true,
  supportsHydration: false,
  supportsMicrotasks: false,
  getRootHostContext: (..._args) => {
    logReconcilerFunction("getRootHostContext");
    return { root: true };
  },
  prepareForCommit: (..._args) => {
    logReconcilerFunction("prepareForCommit");
    return null;
  },
  resetAfterCommit: (..._args) => {
    logReconcilerFunction("resetAfterCommit");
  },
  getChildHostContext: (..._args) => {
    logReconcilerFunction("getChildHostContext");
    return { root: false };
  },
  shouldSetTextContent(type) {
    logReconcilerFunction("shouldSetTextContent", type);
    return false;
  },
  createTextInstance(text, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("createTextInstance", text);
    return {
      type: "RawText",
      text,
    };
  },
  createInstance(type, props: Props, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("createInstance", type);
    const element = {
      id: generateInstanceId(),
      type,
      props: { ...props },
      children: [],
    } as Instance;

    return element;
  },
  appendInitialChild(parentInstance, child) {
    logReconcilerFunction("appendInitialChild");
    parentInstance.children.push(child);
  },
  finalizeInitialChildren(_instance, type) {
    logReconcilerFunction("finalizeInitialChildren", type);
    return true;
  },
  createContainerChildSet: () => {
    logReconcilerFunction("createContainerChildSet");
    return { children: [] };
  },
  appendChildToContainerChildSet: (containerChildSet, child) => {
    logReconcilerFunction("appendChildToContainerChildSet");
    containerChildSet.children.push(child);
  },
  finalizeContainerChildren: (container, newChildren) => {
    logReconcilerFunction("finalizeContainerChildren");
    container.children = newChildren.children;
    buildAndSyncDrawState(container);
  },
  clearContainer(container) {
    logReconcilerFunction("clearContainer");
    container.children = [];
  },
  replaceContainerChildren(container, newChildren) {
    logReconcilerFunction("replaceContainerChildren");
    container.children = newChildren.children;
  },
  prepareUpdate(_instance, type, oldProps, newProps, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("prepareUpdate", type);
    const changes = {
      props: [] as string[],
    };
    const keys = Object.keys({ ...oldProps, ...newProps });
    for (const key of keys) {
      if ((oldProps as Record<string, unknown>)[key] !== (newProps as Record<string, unknown>)[key]) {
        changes.props.push(key);
      }
    }
    return changes.props.length ? { props: changes.props } : null;
  },
  cloneInstance(
    instance,
    _updatePayload,
    type,
    _oldProps: Props,
    newProps: Props,
    _internalInstanceHandle,
    keepChildren,
    _recyclableInstance,
  ) {
    logReconcilerFunction("cloneInstance", type);
    const clonedProps: Props = { ...newProps };

    if (type === "vita-text") {
      return {
        id: instance.id,
        type: "vita-text",
        props: clonedProps as PropsByType["vita-text"],
        children: keepChildren ? [...instance.children] : [],
      } as VitaTextInstance;
    }

    if (type === "vita-rect") {
      return {
        id: instance.id,
        type: "vita-rect",
        props: clonedProps as PropsByType["vita-rect"],
        children: keepChildren ? [...instance.children] : [],
      } as VitaRectInstance;
    }

    if (type === "vita-button") {
      return {
        id: instance.id,
        type: "vita-button",
        props: clonedProps as PropsByType["vita-button"],
        children: [],
      } as VitaButtonInstance;
    }

    exhaustiveGuard(type, `Unsupported element type: ${type}`);
  },
  cloneHiddenInstance(instance, type, props, _internalInstanceHandle) {
    logReconcilerFunction("cloneHiddenInstance", type);
    return {
      id: instance.id,
      type,
      props: { ...props },
      children: [],
    } as Instance;
  },
  cloneHiddenTextInstance(_instance, text, _internalInstanceHandle) {
    logReconcilerFunction("cloneHiddenTextInstance", text);
    return {
      type: "RawText",
      text: text,
    } as TextInstance;
  },
  commitMount(_instance, type) {
    logReconcilerFunction("commitMount", type);
  },
  detachDeletedInstance() {
    logReconcilerFunction("detachDeletedInstance");
  },
} satisfies Partial<VitadeckHostConfig>;

const persistentMetrics = new ReconcilerMetrics("persistent");
const hostConfig = instrumentHostConfig(rawHostConfig, persistentMetrics, "persistent");

export const persistentReconcilerMetrics = persistentMetrics;
export const VitadeckReactReconciler = Reconciler(hostConfig as unknown as VitadeckHostConfig);
