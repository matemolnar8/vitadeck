import Reconciler, { HostConfig } from "react-reconciler";

type TextChild = string | number | boolean | null | undefined;

declare global {
  namespace JSX {
    interface IntrinsicElements {
      "vita-text": {
        children: TextChild | TextChild[];
        color?: Color | undefined;
      };
      "vita-rect": {
        x: number;
        y: number;
        width: number;
        height: number;
        variant?: "fill" | "outline";
        color?: Color | undefined;
        children?: React.ReactNode | React.ReactNode[];
      };
    }
  }
}

type VitadeckElementsProps = Pick<JSX.IntrinsicElements, "vita-text" | "vita-rect">;
export type Type = keyof VitadeckElementsProps;
type PropsByType = { [K in Type]: VitadeckElementsProps[K] };
export type Props = VitadeckElementsProps[keyof VitadeckElementsProps];
export type Container = { children: (Instance | TextInstance)[] };

export type Instance = {
  [K in Type]: {
    type: K;
    props: PropsByType[K];
    children: (Instance | TextInstance)[];
  };
}[Type];

export type TextInstance = {
  type: "RawText";
  text: string;
};
export type SuspenseInstance = never;
export type HydratableInstance = never;
export type PublicInstance = Instance | TextInstance;
export type HostContext = { root: boolean };
export type UpdatePayload = {
  props: string[];
  style: string[];
};
export type ChildSet = {
  children: (Instance | TextInstance)[];
};
export type TimeoutHandle = number;
export type NoTimeout = -1;

export type VitadeckHostConfig = HostConfig<
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

const TRACE = false;

// Renderer based on Tsoding's Murayact https://github.com/tsoding/Murayact/
const hostConfig = {
  noTimeout: -1,
  isPrimaryRenderer: true,
  supportsMutation: false,
  supportsPersistence: true,
  supportsHydration: false,
  supportsMicrotasks: false,
  getRootHostContext: (...args) => {
    if (TRACE) console.log("getRootHostContext", args);
    return { root: true };
  },
  prepareForCommit: (...args) => {
    if (TRACE) console.log("prepareForCommit", args);
    return null;
  },
  resetAfterCommit: (...args) => {
    if (TRACE) console.log("resetAfterCommit", args);
  },
  getChildHostContext: (...args) => {
    if (TRACE) console.log("getChildHostContext", args);
    return { root: false };
  },
  shouldSetTextContent(type, props) {
    if (TRACE) console.log("shouldSetTextContent", type, props);
    // TODO: Maybe set to true for some elements like button
    return false;
  },
  createTextInstance(text, _rootContainerInstance, _hostContext) {
    if (TRACE) console.log("createTextInstance");
    return {
      type: "RawText",
      text,
    };
  },
  createInstance(
    type,
    props: Props,
    _rootContainerInstance,
    _hostContext
  ) {
    if (TRACE) console.log("createInstance");
    const element = {
      type,
      props: { ...props },
      children: [],
    } as unknown as Instance;
    return element;
  },
  appendInitialChild(parentInstance, child) {
    if (TRACE) console.log("appendInitialChild");
    parentInstance.children.push(child);
  },
  finalizeInitialChildren(...args) {
    if (TRACE) console.log("finalizeInitialChildren", args);
    return true;
  },
  createContainerChildSet: (...args) => {
    if (TRACE) console.log("createContainerChildSet", args);
    return { children: [] };
  },
  appendChildToContainerChildSet: (containerChildSet, child) => {
    if (TRACE)
      console.log("appendChildToContainerChildSet", containerChildSet, child);
    containerChildSet.children.push(child);
  },
  finalizeContainerChildren: (...args) => {
    if (TRACE) console.log("finalizeContainerChildren", args);
    return { children: [] };
  },
  clearContainer(rootContainerInstance) {
    if (TRACE) console.log("clearContainer", rootContainerInstance);
    rootContainerInstance.children = [];
  },
  replaceContainerChildren(container, newChildren) {
    if (TRACE) console.log("replaceContainerChildren", container, newChildren);
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
    if (TRACE) console.log("prepareUpdate");
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
      console.log(
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
    if (TRACE) console.log("cloneHiddenInstance", instance, type, props);
    return {
      type,
      props: { ...props },
      children: [],
    } as unknown as Instance;
  },
  cloneHiddenTextInstance(instance, text, _internalInstanceHandle) {
    if (TRACE) console.log("cloneHiddenTextInstance", instance, text);
    return {
      type: "RawText",
      text: text,
    } as TextInstance;
  },
  detachDeletedInstance(instance) {
    if (TRACE) console.log("detachDeletedInstance", instance);
  },
  commitMount(instance, type, newProps) {
    if (TRACE) console.log("commitMount", instance, type, newProps);
  },
  getPublicInstance(instance) {
    if (TRACE) console.log("getPublicInstance", instance);
    return instance;
  },
} satisfies Partial<VitadeckHostConfig>;

export const VitadeckRenderer = Reconciler(
  hostConfig as unknown as VitadeckHostConfig
);
