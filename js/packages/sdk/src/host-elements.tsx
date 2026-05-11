import React from "react";
import type { VitaHostPropsByType, VitaHostType } from "./vitadeck-host-types";

const createHostElement = React.createElement as (
  type: string,
  props: Record<string, unknown> | null,
  ...children: React.ReactNode[]
) => React.ReactElement;

export function vitaHost<K extends VitaHostType>(
  type: K,
  props: VitaHostPropsByType[K] | null,
  ...children: React.ReactNode[]
): React.ReactElement {
  return createHostElement(type, props as Record<string, unknown> | null, ...children);
}
