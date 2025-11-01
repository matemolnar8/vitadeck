type InteractiveRect = {
  id: string;
  x: number;
  y: number;
  width: number;
  height: number;
  onClick?: () => void;
  onMouseDown?: () => void;
  onMouseUp?: () => void;
  onMouseEnter?: () => void;
  onMouseLeave?: () => void;
};

export const interactiveRects: InteractiveRect[] = [];

export function onInputEventFromNative(
  id: string,
  type: "mouseenter" | "mouseleave" | "mousedown" | "mouseup" | "click",
) {
  const handlers = interactiveRects.find((rect) => rect.id === id);
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
