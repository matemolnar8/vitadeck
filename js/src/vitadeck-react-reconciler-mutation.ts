import type { ReactNode } from "react";
import Reconciler, { type HostConfig } from "react-reconciler";
import { registerHandlers, unregisterHandlers, updateHandlers } from "./input";
import { instrumentHostConfig, ReconcilerMetrics } from "./reconciler-metrics";
import { exhaustiveGuard } from "./utils";

const generateRandomSegment = () => Math.random().toString(36).substring(2, 15);
const generateInstanceId = (): string => {
  return generateRandomSegment() + generateRandomSegment() + generateRandomSegment() + generateRandomSegment();
};

type VitadeckElementsProps = Pick<JSX.IntrinsicElements, "vita-text" | "vita-rect" | "vita-button">;
type Type = keyof VitadeckElementsProps;
type Props = VitadeckElementsProps[keyof VitadeckElementsProps] & { key?: string };
type PropsByType = { [K in Type]: VitadeckElementsProps[K] };
type HostContext = { root: boolean };
type UpdatePayload = { props: string[] } | null;

type Instance = {
  id: string;
  type: Type;
};

type TextInstance = {
  id: string;
  type: "RawText";
};

type Container = Record<string, never>;

type SuspenseInstance = never;
type HydratableInstance = never;
type ChildSet = never;
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
  Instance | TextInstance,
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

// Helper to extract color components or defaults
const colorToArgs = (color: Color | undefined, defaultColor: Color): [boolean, number, number, number, number] => {
  if (color) {
    return [true, color.r, color.g, color.b, color.a];
  }
  return [false, defaultColor.r, defaultColor.g, defaultColor.b, defaultColor.a];
};

// Extract event handlers from props
const extractHandlers = (props: Props) => ({
  onClick: (props as { onClick?: () => void }).onClick,
  onMouseDown: (props as { onMouseDown?: () => void }).onMouseDown,
  onMouseUp: (props as { onMouseUp?: () => void }).onMouseUp,
  onMouseEnter: (props as { onMouseEnter?: () => void }).onMouseEnter,
  onMouseLeave: (props as { onMouseLeave?: () => void }).onMouseLeave,
});

// Create native instance based on type
const createNativeInstance = (id: string, type: Type, props: Props): void => {
  if (type === "vita-rect") {
    const p = props as PropsByType["vita-rect"];
    const hasFill = p.variant !== "outline" && !!p.color;
    const hasOutline = !!p.borderColor || (p.variant === "outline" && !!p.color);
    const fillColor = hasFill && p.color ? p.color : Colors.BLANK;
    const outlineColor = p.borderColor || (hasOutline && p.color ? p.color : Colors.DARKGRAY);

    nativeCreateRect(
      id,
      p.x,
      p.y,
      p.width,
      p.height,
      hasFill,
      fillColor.r,
      fillColor.g,
      fillColor.b,
      fillColor.a,
      hasOutline,
      outlineColor.r,
      outlineColor.g,
      outlineColor.b,
      outlineColor.a,
      p.borderRadius || 0,
    );
    registerHandlers(id, extractHandlers(props));
  } else if (type === "vita-text") {
    const p = props as PropsByType["vita-text"];
    const [hasColor, r, g, b, a] = colorToArgs(p.color, Colors.BLACK);
    nativeCreateText(id, p.fontSize || 30, hasColor, r, g, b, a, p.border || false);
  } else if (type === "vita-button") {
    const p = props as PropsByType["vita-button"];
    const color = p.color || Colors.DARKBLUE;
    const textColor = p.textColor || Colors.RAYWHITE;
    nativeCreateButton(
      id,
      p.x,
      p.y,
      p.width,
      p.height,
      color.r,
      color.g,
      color.b,
      color.a,
      p.label,
      20,
      p.borderRadius || 0,
      textColor.r,
      textColor.g,
      textColor.b,
      textColor.a,
    );
    registerHandlers(id, extractHandlers(props));
  } else {
    exhaustiveGuard(type, `Unsupported element type: ${type}`);
  }
};

// Update native instance based on type
const updateNativeInstance = (id: string, type: Type, props: Props): void => {
  if (type === "vita-rect") {
    const p = props as PropsByType["vita-rect"];
    const hasFill = p.variant !== "outline" && !!p.color;
    const hasOutline = !!p.borderColor || (p.variant === "outline" && !!p.color);
    const fillColor = hasFill && p.color ? p.color : Colors.BLANK;
    const outlineColor = p.borderColor || (hasOutline && p.color ? p.color : Colors.DARKGRAY);

    nativeUpdateRect(
      id,
      p.x,
      p.y,
      p.width,
      p.height,
      hasFill,
      fillColor.r,
      fillColor.g,
      fillColor.b,
      fillColor.a,
      hasOutline,
      outlineColor.r,
      outlineColor.g,
      outlineColor.b,
      outlineColor.a,
      p.borderRadius || 0,
    );
    updateHandlers(id, extractHandlers(props));
  } else if (type === "vita-text") {
    const p = props as PropsByType["vita-text"];
    const [hasColor, r, g, b, a] = colorToArgs(p.color, Colors.BLACK);
    nativeUpdateText(id, p.fontSize || 30, hasColor, r, g, b, a, p.border || false);
  } else if (type === "vita-button") {
    const p = props as PropsByType["vita-button"];
    const color = p.color || Colors.DARKBLUE;
    const textColor = p.textColor || Colors.RAYWHITE;
    nativeUpdateButton(
      id,
      p.x,
      p.y,
      p.width,
      p.height,
      color.r,
      color.g,
      color.b,
      color.a,
      p.label,
      20,
      p.borderRadius || 0,
      textColor.r,
      textColor.g,
      textColor.b,
      textColor.a,
    );
    updateHandlers(id, extractHandlers(props));
  } else {
    exhaustiveGuard(type, `Unsupported element type: ${type}`);
  }
};

const rawHostConfig = {
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

  resetAfterCommit: (_container: Container) => {
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
    const id = generateInstanceId();
    nativeCreateRawText(id, text);
    return { id, type: "RawText" } satisfies TextInstance;
  },

  createInstance(type, props: Props, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("createInstance", type);
    const id = generateInstanceId();
    createNativeInstance(id, type, props);
    return { id, type } satisfies Instance;
  },

  appendInitialChild(parentInstance, child) {
    logReconcilerFunction("appendInitialChild");
    nativeAppendChild(parentInstance.id, child.id);
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
    nativeAppendChild(parentInstance.id, child.id);
  },

  appendChildToContainer(_container, child) {
    logReconcilerFunction("appendChildToContainer");
    // Empty string for parentId means root container
    nativeAppendChild("", child.id);
  },

  insertBefore(parentInstance, child, beforeChild) {
    logReconcilerFunction("insertBefore");
    nativeInsertBefore(parentInstance.id, child.id, beforeChild.id);
  },

  insertInContainerBefore(_container, child, beforeChild) {
    logReconcilerFunction("insertInContainerBefore");
    nativeInsertBefore("", child.id, beforeChild.id);
  },

  removeChild(parentInstance, child) {
    logReconcilerFunction("removeChild");
    nativeRemoveChild(parentInstance.id, child.id);
  },

  removeChildFromContainer(_container, child) {
    logReconcilerFunction("removeChildFromContainer");
    nativeRemoveChild("", child.id);
  },

  commitTextUpdate(textInstance, _oldText, newText) {
    logReconcilerFunction("commitTextUpdate", newText);
    nativeUpdateRawText(textInstance.id, newText);
  },

  resetTextContent(_instance) {
    logReconcilerFunction("resetTextContent");
    // Text content is managed via children, no action needed
  },

  commitUpdate(instance, _updatePayload, type, _prevProps, nextProps: Props, _internalInstanceHandle) {
    logReconcilerFunction("commitUpdate", type);
    updateNativeInstance(instance.id, type, nextProps);
  },

  clearContainer(_container) {
    logReconcilerFunction("clearContainer");
    nativeClearContainer();
  },

  commitMount(_instance, type) {
    logReconcilerFunction("commitMount", type);
  },

  detachDeletedInstance(instance) {
    logReconcilerFunction("detachDeletedInstance");
    unregisterHandlers(instance.id);
    nativeDestroyInstance(instance.id);
  },
} satisfies Partial<VitadeckHostConfig>;

const metrics = new ReconcilerMetrics("reconciler");
const hostConfig = instrumentHostConfig(rawHostConfig, metrics, "reconciler");
const reconciler = Reconciler(hostConfig as unknown as VitadeckHostConfig);

const container: Container = {};
let root: ReturnType<typeof reconciler.createContainer> | null = null;

function onError(error: unknown) {
  console.error("[Reconciler] Error:", error);
}

export function render(element: ReactNode): void {
  if (!root) {
    root = reconciler.createContainer(container, 0, null, true, null, "vitadeck_", onError, null);
  }
  reconciler.updateContainer(element, root);
}

export function getMetrics(): ReconcilerMetrics {
  return metrics;
}

export function resetMetrics(): void {
  metrics.reset();
}
