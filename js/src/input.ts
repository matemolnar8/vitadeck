type EventHandlers = {
  onClick?: () => void;
  onMouseDown?: () => void;
  onMouseUp?: () => void;
  onMouseEnter?: () => void;
  onMouseLeave?: () => void;
};

// Map of instance IDs to their event handlers
const handlersById = new Map<string, EventHandlers>();

// Register event handlers for an instance
export function registerHandlers(id: string, handlers: EventHandlers): void {
  // Only store if there are any handlers
  if (handlers.onClick || handlers.onMouseDown || handlers.onMouseUp || 
      handlers.onMouseEnter || handlers.onMouseLeave) {
    handlersById.set(id, handlers);
  }
}

// Update event handlers for an instance
export function updateHandlers(id: string, handlers: EventHandlers): void {
  if (handlers.onClick || handlers.onMouseDown || handlers.onMouseUp || 
      handlers.onMouseEnter || handlers.onMouseLeave) {
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
  type: "mouseenter" | "mouseleave" | "mousedown" | "mouseup" | "click",
): void {
  const handlers = handlersById.get(id);
  if (!handlers) return;
  
  switch (type) {
    case "mouseenter":
      handlers.onMouseEnter?.();
      break;
    case "mouseleave":
      handlers.onMouseLeave?.();
      break;
    case "mousedown":
      handlers.onMouseDown?.();
      break;
    case "mouseup":
      handlers.onMouseUp?.();
      break;
    case "click":
      handlers.onClick?.();
      break;
  }
}
