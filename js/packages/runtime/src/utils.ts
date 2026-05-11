export function exhaustiveGuard(_value: never, message?: string): never {
  throw new Error(message ?? "Unexpected exhaustive case reached");
}
