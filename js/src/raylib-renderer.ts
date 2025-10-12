import {
  Instance,
  TextInstance,
  VitaTextInstance,
  VitaRectInstance,
} from "./vitadeck-react-reconciler";

type RectContext = {
  x: number;
  y: number;
  width: number;
  height: number;
  textIndex: number; // line index for text layout inside this rect
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
  };
  renderVitadeckElement(child.children, childRectCtx);
}

export function renderVitadeckElement(
  children: (Instance | TextInstance)[],
  rectCtx: RectContext = { x: 0, y: 0, width: 960, height: 544, textIndex: 0 }
) {
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
