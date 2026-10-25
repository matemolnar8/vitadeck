import type { Color } from "@vitadeck/sdk/types";
import type { FocusProps, VitaHostPropsByType } from "@vitadeck/sdk/vitadeck-host-types";
import type { ReactNode } from "react";
import Reconciler, { type HostConfig } from "react-reconciler";
import { DefaultEventPriority } from "react-reconciler/constants";
import { registerHandlers, unregisterHandlers, updateHandlers, type EventHandlers } from "./input";
import { instrumentHostConfig, ReconcilerMetrics } from "./reconciler-metrics";
import { exhaustiveGuard } from "./utils";

const generateRandomSegment = () => Math.random().toString(36).slice(2, 15);
const generateInstanceId = (): string => {
  return generateRandomSegment() + generateRandomSegment() + generateRandomSegment() + generateRandomSegment();
};

type Type = keyof VitaHostPropsByType;
type Props = VitaHostPropsByType[Type] & { key?: string };
type PropsByType = { [K in Type]: VitaHostPropsByType[K] };
type HostContext = { root: boolean };
// native: a prop the C instance stores changed; handlers: only the JS input handlers need re-registering.
type UpdatePayload = { native: boolean; handlers: boolean };

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

const handlerKeys = [
  "onPress",
  "onPressStart",
  "onPressEnd",
  "onFocus",
  "onBlur",
  "onChange",
] as const satisfies ReadonlyArray<keyof EventHandlers>;

// Buttons and TextInputs carry handlers; the other host types lack these keys.
const extractHandlers = (props: Props): EventHandlers => {
  const { onPress, onPressStart, onPressEnd, onFocus, onBlur, onChange } = props as EventHandlers;
  return { onPress, onPressStart, onPressEnd, onFocus, onBlur, onChange };
};

const textAlignToNative = (align: PropsByType["vita-text"]["align"]): number => {
  if (align === "center") return 1;
  if (align === "right") return 2;
  return 0;
};

const textWrapToNative = (wrap: PropsByType["vita-text"]["wrap"]): number => {
  return wrap === "word" ? 1 : 0;
};

const textLayoutArgs = (p: PropsByType["vita-text"]) =>
  [p.x ?? 8, p.y ?? 8, p.width ?? 0, p.lineHeight ?? 0, textAlignToNative(p.align), textWrapToNative(p.wrap)] as const;

const textInputArgs = (p: PropsByType["vita-text-input"]) => {
  const color = p.color ?? Colors.RAYWHITE;
  const textColor = p.textColor ?? Colors.BLACK;
  return [
    p.x ?? 0,
    p.y ?? 0,
    p.width,
    p.height,
    color.r,
    color.g,
    color.b,
    color.a,
    textColor.r,
    textColor.g,
    textColor.b,
    textColor.a,
    ...colorToArgs(p.focusColor, Colors.BLANK),
    p.font ?? "default",
    p.fontSize ?? 20,
    p.borderRadius ?? 0,
    p.maxLength ?? 256,
    p.value,
    p.placeholder ?? "",
  ] as const;
};

const stackAlignToNative = (align: PropsByType["vita-stack"]["align"]): number => {
  if (align === "center") return 1;
  if (align === "end") return 2;
  return 0;
};

const stackArgs = (p: PropsByType["vita-stack"]) => {
  const [hasFill, r, g, b, a] = colorToArgs(p.color, Colors.BLANK);
  return [
    p.x ?? 0,
    p.y ?? 0,
    p.width ?? 0,
    p.height ?? 0,
    hasFill,
    r,
    g,
    b,
    a,
    p.gap ?? 0,
    p.padding ?? 0,
    p.direction === "row" ? 1 : 0,
    stackAlignToNative(p.align),
  ] as const;
};

// Create native instance based on type
const createNativeInstance = (id: string, type: Type, props: Props): void => {
  if (type === "vita-rect") {
    const p = props as PropsByType["vita-rect"];
    const hasFill = p.variant !== "outline" && !!p.color;
    const hasOutline = !!p.borderColor || (p.variant === "outline" && !!p.color);
    const fillColor = hasFill && p.color ? p.color : Colors.BLANK;
    const outlineColor = p.borderColor ?? (hasOutline && p.color ? p.color : Colors.DARKGRAY);

    nativeCreateRect(
      id,
      p.x ?? 0,
      p.y ?? 0,
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
      p.borderRadius ?? 0,
    );
    registerHandlers(id, extractHandlers(props));
  } else if (type === "vita-text") {
    const p = props as PropsByType["vita-text"];
    const [hasColor, r, g, b, a] = colorToArgs(p.color, Colors.BLACK);
    const [tx, ty, tw, tlh, talign, twrap] = textLayoutArgs(p);
    nativeCreateText(
      id,
      p.font ?? "default",
      p.fontSize ?? 30,
      hasColor,
      r,
      g,
      b,
      a,
      p.border ?? false,
      tx,
      ty,
      tw,
      tlh,
      talign,
      twrap,
    );
  } else if (type === "vita-button") {
    const p = props as PropsByType["vita-button"];
    const color = p.color ?? Colors.DARKBLUE;
    const textColor = p.textColor ?? Colors.RAYWHITE;
    nativeCreateButton(
      id,
      p.x ?? 0,
      p.y ?? 0,
      p.width,
      p.height,
      color.r,
      color.g,
      color.b,
      color.a,
      p.label,
      20,
      p.borderRadius ?? 0,
      textColor.r,
      textColor.g,
      textColor.b,
      textColor.a,
      ...colorToArgs(p.focusColor, Colors.BLANK),
    );
    registerHandlers(id, extractHandlers(props));
  } else if (type === "vita-text-input") {
    nativeCreateTextInput(id, ...textInputArgs(props as PropsByType["vita-text-input"]));
    registerHandlers(id, extractHandlers(props));
  } else if (type === "vita-scroll") {
    const p = props as PropsByType["vita-scroll"];
    const [hasFill, r, g, b, a] = colorToArgs(p.color, Colors.BLANK);
    nativeCreateScroll(id, p.x, p.y, p.width, p.height, hasFill, r, g, b, a, p.gap ?? 0, p.padding ?? 0);
  } else if (type === "vita-stack") {
    nativeCreateStack(id, ...stackArgs(props as PropsByType["vita-stack"]));
  } else if (type === "vita-image") {
    const p = props as PropsByType["vita-image"];
    nativeCreateImage(id, p.image, p.x ?? 0, p.y ?? 0, p.width ?? 0, p.height ?? 0);
  } else {
    exhaustiveGuard(type, `Unsupported element type: ${String(type)}`);
  }
};

const handlerKeySet: ReadonlySet<string> = new Set(handlerKeys);

const isColor = (value: unknown): value is Color =>
  typeof value === "object" && value !== null && "r" in value && "g" in value && "b" in value && "a" in value;

// Colors are often fresh object literals on every render, so they compare by value.
const samePropValue = (prev: unknown, next: unknown): boolean =>
  prev === next ||
  (isColor(prev) && isColor(next) && prev.r === next.r && prev.g === next.g && prev.b === next.b && prev.a === next.a);

const propRecord = (props: Props): Readonly<Record<string, unknown>> => props;

const diffProps = (oldProps: Props, newProps: Props): UpdatePayload | null => {
  const prev = propRecord(oldProps);
  const next = propRecord(newProps);
  let native = false;
  let handlers = false;
  const compare = (key: string) => {
    // Children are reconciled as their own instances and never reach the native update.
    // autoFocus only matters on mount, through commitMount.
    if (key === "children" || key === "autoFocus") return;
    if (handlerKeySet.has(key)) {
      handlers ||= prev[key] !== next[key];
    } else {
      native ||= !samePropValue(prev[key], next[key]);
    }
  };
  for (const key of Object.keys(next)) compare(key);
  for (const key of Object.keys(prev)) if (!(key in next)) compare(key);
  return native || handlers ? { native, handlers } : null;
};

// Update native instance based on type
const updateNativeInstance = (id: string, type: Type, props: Props): void => {
  if (type === "vita-rect") {
    const p = props as PropsByType["vita-rect"];
    const hasFill = p.variant !== "outline" && !!p.color;
    const hasOutline = !!p.borderColor || (p.variant === "outline" && !!p.color);
    const fillColor = hasFill && p.color ? p.color : Colors.BLANK;
    const outlineColor = p.borderColor ?? (hasOutline && p.color ? p.color : Colors.DARKGRAY);

    nativeUpdateRect(
      id,
      p.x ?? 0,
      p.y ?? 0,
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
      p.borderRadius ?? 0,
    );
  } else if (type === "vita-text") {
    const p = props as PropsByType["vita-text"];
    const [hasColor, r, g, b, a] = colorToArgs(p.color, Colors.BLACK);
    const [tx, ty, tw, tlh, talign, twrap] = textLayoutArgs(p);
    nativeUpdateText(
      id,
      p.font ?? "default",
      p.fontSize ?? 30,
      hasColor,
      r,
      g,
      b,
      a,
      p.border ?? false,
      tx,
      ty,
      tw,
      tlh,
      talign,
      twrap,
    );
  } else if (type === "vita-button") {
    const p = props as PropsByType["vita-button"];
    const color = p.color ?? Colors.DARKBLUE;
    const textColor = p.textColor ?? Colors.RAYWHITE;
    nativeUpdateButton(
      id,
      p.x ?? 0,
      p.y ?? 0,
      p.width,
      p.height,
      color.r,
      color.g,
      color.b,
      color.a,
      p.label,
      20,
      p.borderRadius ?? 0,
      textColor.r,
      textColor.g,
      textColor.b,
      textColor.a,
      ...colorToArgs(p.focusColor, Colors.BLANK),
    );
  } else if (type === "vita-text-input") {
    nativeUpdateTextInput(id, ...textInputArgs(props as PropsByType["vita-text-input"]));
  } else if (type === "vita-scroll") {
    const p = props as PropsByType["vita-scroll"];
    const [hasFill, r, g, b, a] = colorToArgs(p.color, Colors.BLANK);
    nativeUpdateScroll(id, p.x, p.y, p.width, p.height, hasFill, r, g, b, a, p.gap ?? 0, p.padding ?? 0);
  } else if (type === "vita-stack") {
    nativeUpdateStack(id, ...stackArgs(props as PropsByType["vita-stack"]));
  } else if (type === "vita-image") {
    const p = props as PropsByType["vita-image"];
    nativeUpdateImage(id, p.image, p.x ?? 0, p.y ?? 0, p.width ?? 0, p.height ?? 0);
  } else {
    exhaustiveGuard(type, `Unsupported element type: ${String(type)}`);
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

  // Returning true schedules commitMount, which applies autoFocus once the Button or TextInput is committed.
  finalizeInitialChildren(_instance, type, props: Props) {
    logReconcilerFunction("finalizeInitialChildren", type);
    return (type === "vita-button" || type === "vita-text-input") && (props as FocusProps).autoFocus === true;
  },

  prepareUpdate(_instance, type, oldProps, newProps, _rootContainerInstance, _hostContext) {
    logReconcilerFunction("prepareUpdate", type);
    return diffProps(oldProps, newProps);
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

  commitUpdate(instance, updatePayload, type, _prevProps, nextProps: Props, _internalInstanceHandle) {
    logReconcilerFunction("commitUpdate", type);
    if (updatePayload.native) updateNativeInstance(instance.id, type, nextProps);
    if (updatePayload.handlers) updateHandlers(instance.id, extractHandlers(nextProps));
  },

  clearContainer(_container) {
    logReconcilerFunction("clearContainer");
    nativeClearContainer();
  },

  commitMount(instance, type) {
    logReconcilerFunction("commitMount", type);
    nativeRequestFocus(instance.id);
  },

  detachDeletedInstance(instance) {
    logReconcilerFunction("detachDeletedInstance");
    unregisterHandlers(instance.id);
    nativeDestroyInstance(instance.id);
  },

  getPublicInstance(instance) {
    return instance;
  },

  preparePortalMount(_container) {
    logReconcilerFunction("preparePortalMount");
  },

  scheduleTimeout(fn, delay) {
    return setTimeout(() => {
      fn();
    }, delay ?? 0);
  },

  cancelTimeout(id) {
    clearTimeout(id);
  },

  getCurrentEventPriority() {
    return DefaultEventPriority;
  },

  // The members below back React DOM-only features (event handle and scope APIs) that VitaDeck never enables.
  getInstanceFromNode() {
    return null;
  },

  beforeActiveInstanceBlur() {},

  afterActiveInstanceBlur() {},

  prepareScopeUpdate() {},

  getInstanceFromScope() {
    return null;
  },
} satisfies VitadeckHostConfig;

const metrics = new ReconcilerMetrics("reconciler");
const hostConfig = instrumentHostConfig(rawHostConfig, metrics, "reconciler");
const reconciler = Reconciler(hostConfig);

const container: Container = {};
type ReconcilerRoot = object;
let root: ReconcilerRoot | null = null;

// React recovered from these; uncaught errors reach DeckAppErrorBoundary or the native exception reporters.
function onRecoverableError(error: unknown) {
  console.error("[Reconciler] Recoverable error:", error);
}

export function render(element: ReactNode): void {
  root ??= reconciler.createContainer(container, 0, null, true, null, "vitadeck_", onRecoverableError, null);
  reconciler.updateContainer(element, root as Parameters<typeof reconciler.updateContainer>[1]);
}

export function getMetrics(): ReconcilerMetrics {
  return metrics;
}

export function resetMetrics(): void {
  metrics.reset();
}
