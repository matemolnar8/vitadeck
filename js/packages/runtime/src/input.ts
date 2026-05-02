type EventHandlers = {
  onPress?: () => void;
  onPressStart?: () => void;
  onPressEnd?: () => void;
};

type NativeInputEventType = "mouseenter" | "mouseleave" | "mousedown" | "mouseup" | "click";

// Map of instance IDs to their event handlers
const handlersById = new Map<string, EventHandlers>();

// Register event handlers for an instance
export function registerHandlers(id: string, handlers: EventHandlers): void {
  // Only store if there are any handlers
  if (handlers.onPress || handlers.onPressStart || handlers.onPressEnd) {
    handlersById.set(id, handlers);
  }
}

// Update event handlers for an instance
export function updateHandlers(id: string, handlers: EventHandlers): void {
  if (handlers.onPress || handlers.onPressStart || handlers.onPressEnd) {
    handlersById.set(id, handlers);
  } else {
    // No handlers, remove from map
    handlersById.delete(id);
  }
}

// Unregister handlers when instance is destroyed
export function unregisterHandlers(id: string): void {
  handlersById.delete(id);
}

// Called from native when an input event occurs
export function onInputEventFromNative(
  id: string,
  type: NativeInputEventType,
): void {
  const handlers = handlersById.get(id);
  if (!handlers) return;

  switch (type) {
    case "mouseenter":
      break;
    case "mouseleave":
      break;
    case "mousedown":
      handlers.onPressStart?.();
      break;
    case "mouseup":
      handlers.onPressEnd?.();
      break;
    case "click":
      handlers.onPress?.();
      break;
  }
}
