// Author-visible globals come from the SDK; this file adds the runtime-only contract with the C host.
/// <reference types="@vitadeck/sdk/deck-app-env" />
import type * as VitaDeckSdk from "@vitadeck/sdk";
import type { ColorsMap } from "@vitadeck/sdk/types";
import * as React from "react";
import type * as ReactCompilerRuntime from "react-compiler-runtime";

declare global {
  var Colors: ColorsMap;

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
    fontName: string,
    fontSize: number,
    hasColor: boolean,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    border: boolean,
    x: number,
    y: number,
    width: number,
    lineHeight: number,
    align: number,
    wrap: number,
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
    fontName: string,
    fontSize: number,
    borderRadius: number,
    textColorR: number,
    textColorG: number,
    textColorB: number,
    textColorA: number,
    hasFocusColor: boolean,
    focusColorR: number,
    focusColorG: number,
    focusColorB: number,
    focusColorA: number,
    align: number,
  ): void;

  function nativeCreateTextInput(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    textColorR: number,
    textColorG: number,
    textColorB: number,
    textColorA: number,
    hasFocusColor: boolean,
    focusColorR: number,
    focusColorG: number,
    focusColorB: number,
    focusColorA: number,
    fontName: string,
    fontSize: number,
    borderRadius: number,
    maxLength: number,
    value: string,
    placeholder: string,
  ): void;

  function nativeCreateScroll(
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
    gap: number,
    padding: number,
  ): void;

  function nativeCreateStack(
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
    gap: number,
    padding: number,
    direction: number,
    align: number,
  ): void;

  function nativeCreateImage(id: string, imageName: string, x: number, y: number, width: number, height: number): void;

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
    fontName: string,
    fontSize: number,
    hasColor: boolean,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    border: boolean,
    x: number,
    y: number,
    width: number,
    lineHeight: number,
    align: number,
    wrap: number,
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
    fontName: string,
    fontSize: number,
    borderRadius: number,
    textColorR: number,
    textColorG: number,
    textColorB: number,
    textColorA: number,
    hasFocusColor: boolean,
    focusColorR: number,
    focusColorG: number,
    focusColorB: number,
    focusColorA: number,
    align: number,
  ): void;

  function nativeUpdateTextInput(
    id: string,
    x: number,
    y: number,
    width: number,
    height: number,
    colorR: number,
    colorG: number,
    colorB: number,
    colorA: number,
    textColorR: number,
    textColorG: number,
    textColorB: number,
    textColorA: number,
    hasFocusColor: boolean,
    focusColorR: number,
    focusColorG: number,
    focusColorB: number,
    focusColorA: number,
    fontName: string,
    fontSize: number,
    borderRadius: number,
    maxLength: number,
    value: string,
    placeholder: string,
  ): void;

  function nativeUpdateScroll(
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
    gap: number,
    padding: number,
  ): void;

  function nativeUpdateStack(
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
    gap: number,
    padding: number,
    direction: number,
    align: number,
  ): void;

  function nativeUpdateImage(id: string, imageName: string, x: number, y: number, width: number, height: number): void;

  function nativeUpdateRawText(id: string, text: string): void;

  function nativeRequestFocus(id: string): void;

  function nativeClearContainer(): void;

  function nativeEvalActiveDeckAppEntry(): void;
  // Called by `storage` in @vitadeck/sdk.
  function nativeReadDeckAppStorage(): string;
  function nativeWriteDeckAppStorage(json: string): void;
  function nativeReportDeckAppError(message: string, stack: string): void;

  function nativeFetch(
    url: string,
    method: string,
    headers: string[],
    body?: string,
  ): Promise<{
    status: number;
    ok: boolean;
    statusText: string;
    headers: Record<string, string>;
    body: string;
  }>;

  var React: typeof React;
  var reactCompilerRuntime: typeof ReactCompilerRuntime;
  var vitadeckSdk: typeof VitaDeckSdk;
  var vitadeckPackage: {
    register(component: React.ComponentType): void;
  };
}
