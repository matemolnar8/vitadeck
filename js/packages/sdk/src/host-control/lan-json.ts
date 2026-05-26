export type HostControlFailureCode =
  | "malformed_request"
  | "not_found"
  | "method_not_allowed"
  | "payload_too_large"
  | "unsupported_content_type"
  | "unknown_command"
  | "invalid_payload"
  | "unsupported_platform"
  | "command_failed";

export type VitaDeckLanJsonResult<T extends object = Record<string, never>> =
  | ({ ok: true } & T)
  | { ok: false; code: HostControlFailureCode; message: string };

export type HostControlRequest = {
  command: string;
  payload?: unknown;
};

export type HostControlTransportResponse = {
  status: number;
  body: string;
};

export type HostControlTransport = (request: {
  url: string;
  body: string;
  timeoutMs: number;
}) => Promise<HostControlTransportResponse>;
