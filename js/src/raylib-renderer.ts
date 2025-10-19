import {
  Instance,
  TextInstance,
  VitaTextInstance,
  VitaRectInstance,
} from "./vitadeck-react-reconciler";
import { clearInteractiveRects, registerInteractiveRect } from "./input";

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
  const {
    x,
    y,
    width,
    height,
    variant,
    color,
    onClick,
    onMouseDown,
    onMouseUp,
    onMouseEnter,
    onMouseLeave,
  } = child.props;

  if (variant === "outline") {
    drawRectOutline(rectCtx.x + x, rectCtx.y + y, width, height, color);
  } else {
    drawRect(rectCtx.x + x, rectCtx.y + y, width, height, color);
  }

  if (onClick || onMouseDown || onMouseUp || onMouseEnter || onMouseLeave) {
    const rect: InteractiveRect = {
      id: child.id,
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
    registerInteractiveRect(rect);
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
  rectCtx: RectContext = {
    x: 0,
    y: 0,
    width: 960,
    height: 544,
    textIndex: 0,
    root: true,
  }
) {
  if (rectCtx.root) {
    clearInteractiveRects();
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
