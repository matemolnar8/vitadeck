import * as React from "react";
import type * as ReactCompilerRuntime from "react-compiler-runtime";
import type { ColorsMap } from "@vitadeck/sdk/types";
import type * as VitaDeckSdk from "@vitadeck/sdk";

declare global {
  function getTime(): number;

  var Colors: ColorsMap;

  var console: {
    log: (...args: unknown[]) => void;
    debug: (...args: unknown[]) => void;
    error: (...args: unknown[]) => void;
  };
  function setInterval(callback: () => void, delay: number): number;
  function clearInterval(id: number): void;
  function nativeCreateRect(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    hasFill: boolean,
    fillR: number,
    fillG: number,
    fillB: number,
    fillA: number,
    hasOutline: boolean,
    outlineR: number,
    outlineG: number,
    outlineB: number,
    outlineA: number,
    borderRadius: number,
  ): void;

  function nativeCreateText(
    id: string,
    fontSize: number,
    hasColor: boolean,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    border: boolean,
  ): void;

  function nativeCreateButton(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    label: string,
    fontSize: number,
    borderRadius: number,
    textColorR: number,
    textColorG: number,
    textColorB: number,
    textColorA: number,
  ): void;

  function nativeCreateRawText(id: string, text: string): void;

  function nativeAppendChild(parentId: string, childId: string): void;
  function nativeInsertBefore(parentId: string, childId: string, beforeId: string): void;
  function nativeRemoveChild(parentId: string, childId: string): void;
  function nativeDestroyInstance(id: string): void;

  function nativeUpdateRect(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    hasFill: boolean,
    fillR: number,
    fillG: number,
    fillB: number,
    fillA: number,
    hasOutline: boolean,
    outlineR: number,
    outlineG: number,
    outlineB: number,
    outlineA: number,
    borderRadius: number,
  ): void;

  function nativeUpdateText(
    id: string,
    fontSize: number,
    hasColor: boolean,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    border: boolean,
  ): void;

  function nativeUpdateButton(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    label: string,
    fontSize: number,
    borderRadius: number,
    textColorR: number,
    textColorG: number,
    textColorB: number,
    textColorA: number,
  ): void;

  function nativeUpdateRawText(id: string, text: string): void;

  function nativeClearContainer(): void;

  function nativeReadTextFile(path: string): string;
  function nativeEvalFile(path: string): void;

  var React: typeof React;
  var reactCompilerRuntime: typeof ReactCompilerRuntime;
  var vitadeckSdk: typeof VitaDeckSdk;
  var vitadeckPackage: {
    register(component: React.ComponentType): void;
  };
}
