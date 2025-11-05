const generateRandomSegment = () => Math.random().toString(36).substring(2, 15);

export const generateInstanceId = (): string => {
  return generateRandomSegment() + generateRandomSegment() + generateRandomSegment() + generateRandomSegment();
};

type TextChild = string | number | boolean | null | undefined;

type JSXPropsWithKey<T> = T & { key?: string };

declare global {
  namespace JSX {
    interface IntrinsicElements {
      "vita-text": JSXPropsWithKey<{
        children: TextChild | TextChild[];
        color?: Color;
        border?: boolean;
        fontSize?: number;
      }>;
      "vita-rect": JSXPropsWithKey<{
        x: number;
        y: number;
        width: number;
        height: number;
        variant?: "fill" | "outline";
        color?: Color;
        borderColor?: Color;
        onClick?: (() => void) | undefined;
        onMouseDown?: () => void;
        onMouseUp?: () => void;
        onMouseEnter?: () => void;
        onMouseLeave?: () => void;
        children?: React.ReactNode | React.ReactNode[];
      }>;
      "vita-button": JSXPropsWithKey<{
        x: number;
        y: number;
        width: number;
        height: number;
        color?: Color;
        label: string;
        onClick?: () => void;
        onMouseDown?: () => void;
        onMouseUp?: () => void;
        onMouseEnter?: () => void;
        onMouseLeave?: () => void;
      }>;
    }
  }
}

export type VitadeckElementsProps = Pick<JSX.IntrinsicElements, "vita-text" | "vita-rect" | "vita-button">;
export type Type = keyof VitadeckElementsProps;
export type Props = VitadeckElementsProps[keyof VitadeckElementsProps] & {
  key?: string;
};
export type PropsByType = { [K in Type]: VitadeckElementsProps[K] };

export type Instance = {
  [K in Type]: {
    id: string;
    type: K;
    props: PropsByType[K];
    children: (Instance | TextInstance)[];
  };
}[Type];

export type VitaTextInstance = Extract<Instance, { type: "vita-text" }>;
export type VitaRectInstance = Extract<Instance, { type: "vita-rect" }>;
export type VitaButtonInstance = Extract<Instance, { type: "vita-button" }>;

export type TextInstance = {
  type: "RawText";
  text: string;
};

export type VitadeckContainer = { children: (Instance | TextInstance)[] };
export type VitadeckPublicInstance = Instance | TextInstance;
export type HostContext = { root: boolean };
export type UpdatePayload = {
  props: string[];
} | null;
export type ChildSet = {
  children: (Instance | TextInstance)[];
};
