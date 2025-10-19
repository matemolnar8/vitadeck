import {
  Instance,
  TextInstance,
  VitaTextInstance,
  VitaRectInstance,
} from "./vitadeck-react-reconciler";

type InteractiveRect = {
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

const interactiveRects: InteractiveRect[] = [];
let lastHoverIndex: number = -1;

type RectContext = {
  x: number;
  y: number;
  width: number;
  height: number;
  textIndex: number; // line index for text layout inside this rect
  root: boolean;
};

function renderVitaText(child: VitaTextInstance, rectCtx: RectContext) {
  let text = "";
  child.children.forEach((grandChild) => {
    if (grandChild.type === "RawText") {
      text += grandChild.text;
    }
  });
  const padding = 8;
  const fontSize = child.props.fontSize || 30;
  drawText(
    rectCtx.x + padding,
    rectCtx.y + padding + rectCtx.textIndex * fontSize,
    fontSize,
    text,
    child.props.color,
    child.props.border
  );
  rectCtx.textIndex++;
}

function renderVitaRect(child: VitaRectInstance, rectCtx: RectContext) {
  const { x, y, width, height, variant, color, onClick, onMouseDown, onMouseUp, onMouseEnter, onMouseLeave } = child.props;

  if (variant === "outline") {
    drawRectOutline(rectCtx.x + x, rectCtx.y + y, width, height, color);
  } else {
    drawRect(rectCtx.x + x, rectCtx.y + y, width, height, color);
  }

  // Register as interactive if any handler is provided
  if (onClick || onMouseDown || onMouseUp || onMouseEnter || onMouseLeave) {
    const rect: InteractiveRect = {
      x: rectCtx.x + x,
      y: rectCtx.y + y,
      width,
      height,
    };
    if (onClick) rect.onClick = onClick;
    if (onMouseDown) rect.onMouseDown = onMouseDown;
    if (onMouseUp) rect.onMouseUp = onMouseUp;
    if (onMouseEnter) rect.onMouseEnter = onMouseEnter;
    if (onMouseLeave) rect.onMouseLeave = onMouseLeave;
    interactiveRects.push(rect);
  }

  const childRectCtx: RectContext = {
    x: rectCtx.x + x,
    y: rectCtx.y + y,
    width,
    height,
    textIndex: 0,
    root: false,
  };
  renderVitadeckElement(child.children, childRectCtx);
}

export function renderVitadeckElement(
  children: (Instance | TextInstance)[],
  rectCtx: RectContext = { x: 0, y: 0, width: 960, height: 544, textIndex: 0, root: true }
) {
  // Clear interactive rects at the start of a frame render from the root context
  if (rectCtx.root) {
    interactiveRects.length = 0;
  }
  
  for (const child of children) {
    switch (child.type) {
      case "vita-text":
        renderVitaText(child as VitaTextInstance, rectCtx);
        break;
      case "vita-rect":
        renderVitaRect(child as VitaRectInstance, rectCtx);
        break;
      default:
        throw "TODO child.type: " + child.type;
    }
  }
}

export function processClicks() {
  const mx = getMouseX();
  const my = getMouseY();

  // Find topmost rect under pointer
  let currentTopIndex = -1;
  for (let i = interactiveRects.length - 1; i >= 0; i--) {
    const r = interactiveRects[i];
    if (!r) continue;
    if (mx >= r.x && mx < r.x + r.width && my >= r.y && my < r.y + r.height) {
      currentTopIndex = i;
      break;
    }
  }

  // Hover transitions
  if (lastHoverIndex !== currentTopIndex) {
    const prev = lastHoverIndex >= 0 ? interactiveRects[lastHoverIndex] : undefined;
    if (prev?.onMouseLeave) {
      try {
        prev.onMouseLeave();
      } catch (e) {
        console.error(e);
      }
    }
    const currHover = currentTopIndex >= 0 ? interactiveRects[currentTopIndex] : undefined;
    if (currHover?.onMouseEnter) {
      try {
        currHover.onMouseEnter();
      } catch (e) {
        console.error(e);
      }
    }
    lastHoverIndex = currentTopIndex;
  }

  // Mouse down/click on topmost
  if (isMouseButtonPressed(0) && currentTopIndex >= 0) {
    const r = interactiveRects[currentTopIndex];
    if (!r) return;
    try {
      r.onMouseDown?.();
    } catch (e) {
      console.error(e);
    }
    try {
      r.onClick?.();
    } catch (e) {
      console.error(e);
    }
  }

  // Mouse up on topmost
  if (isMouseButtonReleased(0) && currentTopIndex >= 0) {
    const r = interactiveRects[currentTopIndex];
    if (!r) return;
    try {
      r.onMouseUp?.();
    } catch (e) {
      console.error(e);
    }
  }
}
