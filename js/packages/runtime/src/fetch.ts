export type VitaHeadersInit = Record<string, string> | Array<[string, string]>;

export interface VitaRequestInit {
  method?: string;
  headers?: VitaHeadersInit;
  body?: string;
}

export interface VitaResponse {
  readonly status: number;
  readonly ok: boolean;
  readonly statusText: string;
  readonly headers: Record<string, string>;
  text(): Promise<string>;
  json<T = unknown>(): Promise<T>;
}

function normalizeHeaders(init?: VitaHeadersInit): string[] {
  if (!init) return [];
  const entries = Array.isArray(init) ? init : Object.entries(init);
  return entries.map(([key, value]) => `${key}: ${value}`);
}

class Response implements VitaResponse {
  readonly status: number;
  readonly ok: boolean;
  readonly statusText: string;
  readonly headers: Record<string, string>;
  readonly #body: string;

  constructor(native: Awaited<ReturnType<typeof nativeFetch>>) {
    this.status = native.status;
    this.ok = native.ok;
    this.statusText = native.statusText;
    this.headers = native.headers;
    this.#body = native.body;
  }

  text(): Promise<string> {
    return Promise.resolve(this.#body);
  }

  json<T = unknown>(): Promise<T> {
    return Promise.resolve(JSON.parse(this.#body) as T);
  }
}

export async function fetch(url: string, init?: VitaRequestInit): Promise<VitaResponse> {
  const method = (init?.method ?? "GET").toUpperCase();
  const headers = normalizeHeaders(init?.headers);
  const native = await nativeFetch(url, method, headers, init?.body);
  return new Response(native);
}

export function installFetch(): void {
  globalThis.fetch = fetch;
}
