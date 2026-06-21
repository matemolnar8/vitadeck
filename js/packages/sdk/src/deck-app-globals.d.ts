function setTimeout(callback: () => void, delay: number): number;
function clearTimeout(id: number): void;
function setInterval(callback: () => void, delay: number): number;
function clearInterval(id: number): void;

function getTime(): number;

type VitaHeadersInit = Record<string, string> | Array<[string, string]>;

interface VitaRequestInit {
  method?: string;
  headers?: VitaHeadersInit;
  body?: string;
}

interface VitaResponse {
  readonly status: number;
  readonly ok: boolean;
  readonly statusText: string;
  readonly headers: Record<string, string>;
  text(): Promise<string>;
  json<T = unknown>(): Promise<T>;
}

function fetch(url: string, init?: VitaRequestInit): Promise<VitaResponse>;

declare var console: {
  log: (...args: unknown[]) => void;
  info: (...args: unknown[]) => void;
  debug: (...args: unknown[]) => void;
  warn: (...args: unknown[]) => void;
  error: (...args: unknown[]) => void;
};
