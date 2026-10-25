import { dispatchPadButton, type PadButton, type PadButtonPhase } from "@vitadeck/sdk/internal";

type EventHandlers = {
  onPress?: () => void;
  onPressStart?: () => void;
  onPressEnd?: () => void;
  onFocus?: () => void;
  onBlur?: () => void;
};

type NativeInputEventType = "focus" | "blur" | "pressstart" | "pressend" | "press";

const handlersById = new Map<string, EventHandlers>();

const hasHandlers = (handlers: EventHandlers) => Object.values(handlers).some((handler) => handler !== undefined);

export function registerHandlers(id: string, handlers: EventHandlers): void {
  if (hasHandlers(handlers)) handlersById.set(id, handlers);
}

export function updateHandlers(id: string, handlers: EventHandlers): void {
  if (hasHandlers(handlers)) {
    handlersById.set(id, handlers);
  } else {
    handlersById.delete(id);
  }
}

export function unregisterHandlers(id: string): void {
  handlersById.delete(id);
}

export function onInputEventFromNative(id: string, type: NativeInputEventType): void {
  const handlers = handlersById.get(id);
  if (!handlers) return;

  switch (type) {
    case "focus":
      handlers.onFocus?.();
      break;
    case "blur":
      handlers.onBlur?.();
      break;
    case "pressstart":
      handlers.onPressStart?.();
      break;
    case "pressend":
      handlers.onPressEnd?.();
      break;
    case "press":
      handlers.onPress?.();
      break;
  }
}

export function onPadButtonFromNative(button: PadButton, phase: PadButtonPhase): void {
  dispatchPadButton(button, phase);
}
