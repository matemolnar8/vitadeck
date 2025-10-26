import { exhaustiveGuard } from "./utils";
import type {
  Instance,
  TextInstance,
  VitaButtonInstance,
  VitaRectInstance,
  VitaTextInstance,
} from "./vitadeck-react-reconciler";

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
    child.props.border,
  );
  rectCtx.textIndex++;
}

function renderVitaRect(child: VitaRectInstance, rectCtx: RectContext) {
  const { x, y, width, height, variant, color } = child.props;

  if (variant === "outline") {
    drawRectOutline(rectCtx.x + x, rectCtx.y + y, width, height, color);
  } else {
    drawRect(rectCtx.x + x, rectCtx.y + y, width, height, color);
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

function mixColor(color: Color, mixWith: Color, amount: number): Color {
  return {
    r: Math.round(color.r + (mixWith.r - color.r) * amount),
    g: Math.round(color.g + (mixWith.g - color.g) * amount),
    b: Math.round(color.b + (mixWith.b - color.b) * amount),
    a: color.a,
  };
}

function renderVitaButton(child: VitaButtonInstance, rectCtx: RectContext) {
  const { x, y, width, height, color = Colors.DARKBLUE, label } = child.props;

  const state = getInteractiveState(child.id);
  const hoveredColor = mixColor(color, Colors.WHITE, 0.4);
  const pressedColor = mixColor(color, Colors.BLACK, 0.5);
  const visual = state.pressed ? pressedColor : state.hovered ? hoveredColor : color;

  drawRect(rectCtx.x + x, rectCtx.y + y, width, height, visual);
  const padding = 8;
  const fontSize = 20;
  drawText(rectCtx.x + x + padding, rectCtx.y + y + padding, fontSize, label, Colors.RAYWHITE, false);
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
  },
) {
  for (const child of children) {
    switch (child.type) {
      case "vita-text":
        renderVitaText(child as VitaTextInstance, rectCtx);
        break;
      case "vita-rect":
        renderVitaRect(child as VitaRectInstance, rectCtx);
        break;
      case "vita-button":
        renderVitaButton(child as VitaButtonInstance, rectCtx);
        break;
      case "RawText":
        // Raw text is handled inside vita-text; ignore if encountered at the root.
        break;
      default:
        exhaustiveGuard(child, `Unsupported node type: ${(child as unknown as Instance)?.type}`);
    }
  }
}
