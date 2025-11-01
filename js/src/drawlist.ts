import { interactiveRects } from "./input";
import type {
  Instance,
  TextInstance,
  VitaButtonInstance,
  VitadeckContainer,
  VitaRectInstance,
  VitaTextInstance,
} from "./vitadeck-react-reconciler";

export type DrawRectCommand = {
  type: "rect";
  x: number;
  y: number;
  width: number;
  height: number;
  fillColor?: Color;
  outlineColor?: Color;
};

export type DrawTextCommand = {
  type: "text";
  x: number;
  y: number;
  fontSize: number;
  text: string;
  color?: Color;
  border?: boolean;
};

export type DrawButtonCommand = {
  type: "button";
  id: string;
  x: number;
  y: number;
  width: number;
  height: number;
  baseColor: Color;
  label: string;
  fontSize?: number;
};

export type DrawCommand = DrawRectCommand | DrawTextCommand | DrawButtonCommand;

export const commands: DrawCommand[] = [];

type RectContext = {
  x: number;
  y: number;
  textIndex: number;
};

function pushRectCommand(x: number, y: number, width: number, height: number, fillColor?: Color, outlineColor?: Color) {
  const cmd: DrawRectCommand = {
    type: "rect",
    x,
    y,
    width,
    height,
    ...(fillColor ? { fillColor } : {}),
    ...(outlineColor ? { outlineColor } : {}),
  };
  commands.push(cmd);
}

function pushTextCommand(x: number, y: number, fontSize: number, text: string, color?: Color, border?: boolean) {
  const cmd: DrawTextCommand = {
    type: "text",
    x,
    y,
    fontSize,
    text,
    ...(color ? { color } : {}),
    ...(border ? { border } : {}),
  };
  commands.push(cmd);
}

function pushButtonCommand(
  id: string,
  x: number,
  y: number,
  width: number,
  height: number,
  baseColor: Color,
  label: string,
  fontSize?: number,
) {
  const cmd: DrawButtonCommand = {
    type: "button",
    id,
    x,
    y,
    width,
    height,
    baseColor,
    label,
    ...(fontSize ? { fontSize } : {}),
  };
  commands.push(cmd);
}

export function buildAndSyncDrawState(container: VitadeckContainer) {
  commands.length = 0;
  interactiveRects.length = 0;

  const traverse = (nodes: (Instance | TextInstance)[], ctx: RectContext) => {
    for (const node of nodes) {
      if (!node) continue;
      switch (node.type) {
        case "vita-rect": {
          const rect = node as VitaRectInstance;
          const { x, y, width, height, variant, color, borderColor } = rect.props;
          const absX = ctx.x + x;
          const absY = ctx.y + y;

          const fillColor = variant !== "outline" && color ? color : undefined;
          const outlineColor = borderColor || (variant === "outline" && color ? color : undefined);
          pushRectCommand(absX, absY, width, height, fillColor, outlineColor);

          // Interactive hit area (if any handlers provided)
          const { onClick, onMouseDown, onMouseUp, onMouseEnter, onMouseLeave } =
            rect.props as VitaRectInstance["props"];
          if (onClick || onMouseDown || onMouseUp || onMouseEnter || onMouseLeave) {
            interactiveRects.push({
              id: rect.id,
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

          // Recurse into children with a new rect-local context
          traverse(rect.children || [], { x: absX, y: absY, textIndex: 0 });
          break;
        }
        case "vita-text": {
          const textNode = node as VitaTextInstance;
          let text = "";
          for (const gc of textNode.children) {
            if (gc.type === "RawText") text += (gc as TextInstance).text;
          }
          const padding = 8;
          const fontSize = textNode.props.fontSize || 30;
          const textX = ctx.x + padding;
          const textY = ctx.y + padding + ctx.textIndex * fontSize;
          pushTextCommand(textX, textY, fontSize, text, textNode.props.color, textNode.props.border);
          ctx.textIndex++;
          break;
        }
        case "vita-button": {
          const btn = node as VitaButtonInstance;
          const {
            x,
            y,
            width,
            height,
            color = Colors.DARKBLUE,
            label,
            onClick,
            onMouseDown,
            onMouseUp,
            onMouseEnter,
            onMouseLeave,
          } = btn.props;
          const absX = ctx.x + x;
          const absY = ctx.y + y;
          pushButtonCommand(btn.id, absX, absY, width, height, color, label, 20);

          // Button is interactive if any handler exists
          if (onClick || onMouseDown || onMouseUp || onMouseEnter || onMouseLeave) {
            interactiveRects.push({
              id: btn.id,
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
          break;
        }
        case "RawText":
          // Raw text is consumed by vita-text; ignore at this level
          break;
        default:
          break;
      }
    }
  };

  traverse(container.children || [], { x: 0, y: 0, textIndex: 0 });

  // Sync both interactive rects and draw commands to native side
  syncInteractiveRectsToNative();
  syncDrawListToNative();
}
