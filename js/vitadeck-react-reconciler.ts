import Reconciler, { HostConfig } from "react-reconciler";

// =============================================================================
// Element and Instance Types
// =============================================================================

type TextChild = string | number | boolean | null | undefined;

// JSX element declarations
type JSXPropsWithKey<T> = T & { key?: string };
declare global {
  namespace JSX {
    interface IntrinsicElements {
      "vita-text": JSXPropsWithKey<{
        children: TextChild | TextChild[];
        color?: Color;
        border?: boolean;
        fontSize?: number;
      }>;
      "vita-rect": JSXPropsWithKey<{
        x: number;
        y: number;
        width: number;
        height: number;
        variant?: "fill" | "outline";
        color?: Color;
        children?: React.ReactNode | React.ReactNode[];
      }>;
    }
  }
}

// Element props and type mappings
type VitadeckElementsProps = Pick<
  JSX.IntrinsicElements,
  "vita-text" | "vita-rect"
>;
type Type = keyof VitadeckElementsProps;
type Props = VitadeckElementsProps[keyof VitadeckElementsProps] & {
  key?: string;
};
type PropsByType = { [K in Type]: VitadeckElementsProps[K] };

// Core instance types
export type Instance = {
  [K in Type]: {
    type: K;
    props: PropsByType[K];
    children: (Instance | TextInstance)[];
  };
}[Type];

export type VitaTextInstance = Extract<Instance, { type: "vita-text" }>;
export type VitaRectInstance = Extract<Instance, { type: "vita-rect" }>;

export type TextInstance = {
  type: "RawText";
  text: string;
};

// =============================================================================
// React Reconciler Infrastructure Types
// =============================================================================

type Container = { children: (Instance | TextInstance)[] };
type SuspenseInstance = never;
type HydratableInstance = never;
type PublicInstance = Instance | TextInstance;
type HostContext = { root: boolean };
type UpdatePayload = {
  props: string[];
};
type ChildSet = {
  children: (Instance | TextInstance)[];
};
type TimeoutHandle = number;
type NoTimeout = -1;

type VitadeckHostConfig = HostConfig<
  Type,
  Props,
  Container,
  Instance,
  TextInstance,
  SuspenseInstance,
  HydratableInstance,
  PublicInstance,
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

const TRACE = true;
const logReconcilerFunction = (name: string, ...args: any[]) => {
  if (TRACE) console.debug(`[Reconciler]: ${name}`);
};

// React reconciler host configuration
const hostConfig = {
  noTimeout: -1,
  isPrimaryRenderer: true,
  supportsMutation: false,
  supportsPersistence: true,
  supportsHydration: false,
  supportsMicrotasks: false,
  getRootHostContext: (...args) => {
    logReconcilerFunction("getRootHostContext", args);
    return { root: true };
  },
  prepareForCommit: (...args) => {
    logReconcilerFunction("prepareForCommit", args);
    return null;
  },
  resetAfterCommit: (...args) => {
    logReconcilerFunction("resetAfterCommit", args);
  },
  getChildHostContext: (...args) => {
    logReconcilerFunction("getChildHostContext", args);
    return { root: false };
  },
  shouldSetTextContent(type, props) {
    logReconcilerFunction("shouldSetTextContent", type, props);
    // TODO: Maybe set to true for some elements like button
    return false;
  },
  createTextInstance(text, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("createTextInstance");
    return {
      type: "RawText",
      text,
    };
  },
  createInstance(type, props: Props, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("createInstance");
    const element = {
      type,
      props: { ...props },
      children: [],
    } as unknown as Instance;
    return element;
  },
  appendInitialChild(parentInstance, child) {
    logReconcilerFunction("appendInitialChild");
    parentInstance.children.push(child);
  },
  finalizeInitialChildren(...args) {
    logReconcilerFunction("finalizeInitialChildren", args);
    return true;
  },
  createContainerChildSet: (...args) => {
    logReconcilerFunction("createContainerChildSet", args);
    return { children: [] };
  },
  appendChildToContainerChildSet: (containerChildSet, child) => {
    logReconcilerFunction(
      "appendChildToContainerChildSet",
      containerChildSet,
      child
    );
    containerChildSet.children.push(child);
  },
  finalizeContainerChildren: (container, newChildren) => {
    logReconcilerFunction("finalizeContainerChildren", container, newChildren);
    container.children = newChildren.children;
  },
  clearContainer(rootContainerInstance) {
    logReconcilerFunction("clearContainer", rootContainerInstance);
    rootContainerInstance.children = [];
  },
  replaceContainerChildren(container, newChildren) {
    logReconcilerFunction("replaceContainerChildren", container, newChildren);
    container.children = newChildren.children;
  },
  prepareUpdate(
    _instance,
    _type,
    oldProps,
    newProps,
    _rootContainerInstance,
    _hostContext
  ) {
    logReconcilerFunction("prepareUpdate");
    const changes = {
      props: [] as string[],
    };
    const keys = Object.keys({ ...oldProps, ...newProps });
    for (let key of keys) {
      if (
        (oldProps as Record<string, unknown>)[key] !==
        (newProps as Record<string, unknown>)[key]
      ) {
        changes.props.push(key);
      }
    }
    return changes.props.length ? { props: changes.props } : null;
  },
  cloneInstance(
    instance,
    updatePayload,
    type,
    oldProps: Props,
    newProps: Props,
    _internalInstanceHandle,
    keepChildren
  ) {
    logReconcilerFunction(
      "cloneInstance",
      instance,
      updatePayload,
      type,
      oldProps,
      newProps
    );
    const clonedProps: any = { ...newProps } as any;
    for (let prop of updatePayload.props) {
      if (prop !== "children") {
        (clonedProps as any)[prop] = (newProps as any)[prop];
      }
    }
    return {
      type,
      props: clonedProps,
      children: keepChildren ? [...instance.children] : [],
    } as unknown as Instance;
  },
  cloneHiddenInstance(instance, type, props, _internalInstanceHandle) {
    logReconcilerFunction("cloneHiddenInstance", instance, type, props);
    return {
      type,
      props: { ...props },
      children: [],
    } as unknown as Instance;
  },
  cloneHiddenTextInstance(instance, text, _internalInstanceHandle) {
    logReconcilerFunction("cloneHiddenTextInstance", instance, text);
    return {
      type: "RawText",
      text: text,
    } as TextInstance;
  },
  commitMount(instance, type, newProps) {
    logReconcilerFunction("commitMount", instance, type, newProps);
  },
  detachDeletedInstance(instance) {
    logReconcilerFunction("detachDeletedInstance", instance);
  },
} satisfies Partial<VitadeckHostConfig>;

export const VitadeckReactReconciler = Reconciler(
  hostConfig as unknown as VitadeckHostConfig
);
