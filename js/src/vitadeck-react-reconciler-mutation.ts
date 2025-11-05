import Reconciler, { type HostConfig } from "react-reconciler";
import { buildAndSyncDrawState } from "./drawlist";
import { exhaustiveGuard } from "./utils";
import {
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
type ChildSet = never;
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

const TRACE = false;
const logReconcilerFunction = (name: string, ...args: unknown[]) => {
  if (TRACE) console.log(`[MutationReconciler]: ${name}`, ...args);
};

const hostConfig = {
  noTimeout: -1,
  isPrimaryRenderer: true,
  supportsMutation: true,
  supportsPersistence: false,
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
  resetAfterCommit: (container: VitadeckContainer) => {
    logReconcilerFunction("resetAfterCommit");
    buildAndSyncDrawState(container);
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
    } satisfies TextInstance;
  },
  createInstance(type, props: Props, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("createInstance", type);
    return {
      id: generateInstanceId(),
      type,
      props: { ...props },
      children: [],
    } as Instance;
  },
  appendInitialChild(parentInstance, child) {
    logReconcilerFunction("appendInitialChild");
    parentInstance.children.push(child);
  },
  finalizeInitialChildren(_instance, type) {
    logReconcilerFunction("finalizeInitialChildren", type);
    return true;
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
  appendChild(parentInstance, child) {
    logReconcilerFunction("appendChild");
    parentInstance.children.push(child);
  },
  appendChildToContainer(container, child) {
    logReconcilerFunction("appendChildToContainer");
    container.children.push(child);
  },
  insertBefore(parentInstance, child, beforeChild) {
    logReconcilerFunction("insertBefore");
    const index = parentInstance.children.indexOf(beforeChild as Instance | TextInstance);
    if (index === -1) {
      parentInstance.children.push(child);
      return;
    }
    parentInstance.children.splice(index, 0, child);
  },
  insertInContainerBefore(container, child, beforeChild) {
    logReconcilerFunction("insertInContainerBefore");
    const index = container.children.indexOf(beforeChild as Instance | TextInstance);
    if (index === -1) {
      container.children.push(child);
      return;
    }
    container.children.splice(index, 0, child);
  },
  removeChild(parentInstance, child) {
    logReconcilerFunction("removeChild");
    const index = parentInstance.children.indexOf(child as Instance | TextInstance);
    if (index !== -1) {
      parentInstance.children.splice(index, 1);
    }
  },
  removeChildFromContainer(container, child) {
    logReconcilerFunction("removeChildFromContainer");
    const index = container.children.indexOf(child as Instance | TextInstance);
    if (index !== -1) {
      container.children.splice(index, 1);
    }
  },
  commitTextUpdate(textInstance, _oldText, newText) {
    logReconcilerFunction("commitTextUpdate", newText);
    textInstance.text = newText;
  },
  resetTextContent(instance) {
    logReconcilerFunction("resetTextContent");
    instance.children = [];
  },
  commitUpdate(instance, _updatePayload, type, _prevProps, nextProps: Props, _internalInstanceHandle) {
    logReconcilerFunction("commitUpdate", type);
    const clonedProps: Props = { ...nextProps };

    if (type === "vita-text") {
      (instance as VitaTextInstance).props = clonedProps as PropsByType["vita-text"];
      return;
    }

    if (type === "vita-rect") {
      (instance as VitaRectInstance).props = clonedProps as PropsByType["vita-rect"];
      return;
    }

    if (type === "vita-button") {
      (instance as VitaButtonInstance).props = clonedProps as PropsByType["vita-button"];
      return;
    }

    exhaustiveGuard(type, `Unsupported element type: ${type}`);
  },
  clearContainer(container) {
    logReconcilerFunction("clearContainer");
    container.children = [];
  },
  commitMount(_instance, type) {
    logReconcilerFunction("commitMount", type);
  },
  detachDeletedInstance() {
    logReconcilerFunction("detachDeletedInstance");
  },
} satisfies Partial<VitadeckHostConfig>;

export const VitadeckReactMutationReconciler = Reconciler(hostConfig as unknown as VitadeckHostConfig);
