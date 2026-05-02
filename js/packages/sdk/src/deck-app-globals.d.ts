export {};

declare global {
  function setTimeout(callback: () => void, delay: number): number;
  function clearTimeout(id: number): void;
  function setInterval(callback: () => void, delay: number): number;
  function clearInterval(id: number): void;

  function getTime(): number;

  var console: {
    log: (...args: unknown[]) => void;
    info: (...args: unknown[]) => void;
    debug: (...args: unknown[]) => void;
    warn: (...args: unknown[]) => void;
    error: (...args: unknown[]) => void;
  };
}
