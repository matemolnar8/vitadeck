import { Instance, TextInstance } from "./react-renderer";

const TRACE = false;

type RectContext = {
  x: number;
  y: number;
  width: number;
  height: number;
  textIndex: number; // line index for text layout inside this rect
};

export function renderVitadeckElement(
  children: (Instance | TextInstance)[],
  rectCtx: RectContext = { x: 0, y: 0, width: 960, height: 544, textIndex: 0 }
) {
  if (TRACE) debug("renderVitadeckElement: " + JSON.stringify(children));
  for (const child of children) {
    if (TRACE) debug("child: " + JSON.stringify(child));

    if (child.type === "vita-text") {
      let text = "";
      child.children.forEach((grandChild) => {
        if (grandChild.type === "RawText") {
          text += grandChild.text;
        }
      });
      const padding = 8;
      const fontSize = 30; // align with native default; could be prop later
      drawText(
        rectCtx.x + padding,
        rectCtx.y + padding + rectCtx.textIndex * fontSize,
        fontSize,
        text,
        child.props.color
      );
      rectCtx.textIndex++;
      continue;
    }

    if (child.type === "vita-rect") {
      const { x, y, width, height, variant, color } = child.props;

      if (variant === "outline") {
        drawRectOutline(x, y, width, height, color);
      } else {
        drawRect(x, y, width, height, color);
      }

      const childRectCtx: RectContext = {
        x,
        y,
        width,
        height,
        textIndex: 0,
      };
      renderVitadeckElement(child.children, childRectCtx);
      continue;
    }

    throw "TODO child.type: " + child.type;
  }
}
