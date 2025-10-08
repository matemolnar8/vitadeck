import * as React from "react";

declare global {
  function print(message: string): void;
  function debug(...args: any[]): void;

  function setTimeout(callback: () => void, delay: number): void;

  var console: {
    log: (...args: any[]) => void;
    debug: (...args: any[]) => void;
    warn: (...args: any[]) => void;
    error: (...args: any[]) => void;
  };
}
