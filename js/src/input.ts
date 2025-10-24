import {
  VitadeckContainer,
  VitadeckPublicInstance,
} from "./vitadeck-react-reconciler";

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

export function syncInteractiveRectsFromContainer(
  container: VitadeckContainer
) {
  interactiveRects.length = 0;

  const traverse = (
    nodes: VitadeckPublicInstance[],
    offsetX: number,
    offsetY: number
  ) => {
    for (const node of nodes) {
      if (!node) continue;
      if (node.type === "vita-rect" || node.type === "vita-button") {
        const {
          x,
          y,
          width,
          height,
          onClick,
          onMouseDown,
          onMouseUp,
          onMouseEnter,
          onMouseLeave,
        } = node.props;

        const absX = offsetX + x;
        const absY = offsetY + y;

        if (
          onClick ||
          onMouseDown ||
          onMouseUp ||
          onMouseEnter ||
          onMouseLeave
        ) {
          interactiveRects.push({
            id: node.id,
            x: absX,
            y: absY,
            width,
            height,
            onClick,
            onMouseDown,
            onMouseUp,
            onMouseEnter,
            onMouseLeave,
          });
        }

        // Recurse into children; they render on top
        traverse(node.children || [], absX, absY);
      }
    }
  };

  traverse(container.children || [], 0, 0);

  syncInteractiveRectsToNative();
}

export function onInputEventFromNative(
  id: string,
  type: "mouseenter" | "mouseleave" | "mousedown" | "mouseup" | "click"
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
