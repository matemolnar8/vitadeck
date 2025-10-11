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
  style: string[];
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

const TRACE = false;

// React reconciler host configuration
const hostConfig = {
  noTimeout: -1,
  isPrimaryRenderer: true,
  supportsMutation: false,
  supportsPersistence: true,
  supportsHydration: false,
  supportsMicrotasks: false,
  getRootHostContext: (...args) => {
    if (TRACE) console.debug("getRootHostContext", args);
    return { root: true };
  },
  prepareForCommit: (...args) => {
    if (TRACE) console.debug("prepareForCommit", args);
    return null;
  },
  resetAfterCommit: (...args) => {
    if (TRACE) console.debug("resetAfterCommit", args);
  },
  getChildHostContext: (...args) => {
    if (TRACE) console.debug("getChildHostContext", args);
    return { root: false };
  },
  shouldSetTextContent(type, props) {
    if (TRACE) console.debug("shouldSetTextContent", type, props);
    // TODO: Maybe set to true for some elements like button
    return false;
  },
  createTextInstance(text, _rootContainerInstance, _hostContext) {
    if (TRACE) console.debug("createTextInstance");
    return {
      type: "RawText",
      text,
    };
  },
  createInstance(type, props: Props, _rootContainerInstance, _hostContext) {
    if (TRACE) console.debug("createInstance");
    const element = {
      type,
      props: { ...props },
      children: [],
    } as unknown as Instance;
    return element;
  },
  appendInitialChild(parentInstance, child) {
    if (TRACE) console.debug("appendInitialChild");
    parentInstance.children.push(child);
  },
  finalizeInitialChildren(...args) {
    if (TRACE) console.debug("finalizeInitialChildren", args);
    return true;
  },
  createContainerChildSet: (...args) => {
    if (TRACE) console.debug("createContainerChildSet", args);
    return { children: [] };
  },
  appendChildToContainerChildSet: (containerChildSet, child) => {
    if (TRACE)
      console.debug("appendChildToContainerChildSet", containerChildSet, child);
    containerChildSet.children.push(child);
  },
  finalizeContainerChildren: (...args) => {
    if (TRACE) console.debug("finalizeContainerChildren", args);
    return { children: [] };
  },
  clearContainer(rootContainerInstance) {
    if (TRACE) console.debug("clearContainer", rootContainerInstance);
    rootContainerInstance.children = [];
  },
  replaceContainerChildren(container, newChildren) {
    if (TRACE)
      console.debug("replaceContainerChildren", container, newChildren);
    container.children = newChildren.children;
  },
  prepareUpdate(
    instance,
    type,
    oldProps,
    newProps,
    _rootContainerInstance,
    _hostContext
  ) {
    if (TRACE) console.debug("prepareUpdate");
    const changes = {
      props: [] as string[],
      style: [] as string[],
    };
    const oldP: any = oldProps as any;
    const newP: any = newProps as any;
    for (let key in { ...oldP, ...newP }) {
      if (oldP[key] !== newP[key]) {
        changes.props.push(key);
      }
    }
    for (let key in { ...(oldP.style || {}), ...(newP.style || {}) }) {
      if ((oldP.style || {})[key] !== (newP.style || {})[key]) {
        changes.style.push(key);
      }
    }
    // const updatePayload = changes.props.length || changes.style.length ? { changes } : null;
    return changes;
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
    if (TRACE)
      console.debug(
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
    if (TRACE) console.debug("cloneHiddenInstance", instance, type, props);
    return {
      type,
      props: { ...props },
      children: [],
    } as unknown as Instance;
  },
  cloneHiddenTextInstance(instance, text, _internalInstanceHandle) {
    if (TRACE) console.debug("cloneHiddenTextInstance", instance, text);
    return {
      type: "RawText",
      text: text,
    } as TextInstance;
  },
  detachDeletedInstance(instance) {
    if (TRACE) console.debug("detachDeletedInstance", instance);
  },
  commitMount(instance, type, newProps) {
    if (TRACE) console.debug("commitMount", instance, type, newProps);
  },
  getPublicInstance(instance) {
    if (TRACE) console.debug("getPublicInstance", instance);
    return instance;
  },
} satisfies Partial<VitadeckHostConfig>;

export const VitadeckReactReconciler = Reconciler(
  hostConfig as unknown as VitadeckHostConfig
);
