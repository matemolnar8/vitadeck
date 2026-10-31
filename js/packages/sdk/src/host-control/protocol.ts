/** Bumped only by a breaking change to the Host Control wire contract. */
export const HOST_CONTROL_PROTOCOL_VERSION = 1;

/** Codes a Host Companion or the VitaDeck link endpoint answers with, each with a fixed HTTP status. */
export const HOST_CONTROL_WIRE_ERRORS = {
  malformed_request: 400,
  unsupported_protocol_version: 400,
  forbidden: 403,
  unknown_plugin: 404,
  unknown_command: 404,
  payload_too_large: 413,
  invalid_payload: 422,
  command_failed: 500,
  unsupported_platform: 501,
} as const;

export type HostControlWireErrorCode = keyof typeof HOST_CONTROL_WIRE_ERRORS;

/** Codes only the Deck App side produces; no companion sends them. */
export type HostControlClientErrorCode = "not_linked" | "unreachable" | "timeout" | "bad_response";

export type HostControlErrorCode = HostControlWireErrorCode | HostControlClientErrorCode;

export type HostFailure = { ok: false; code: HostControlErrorCode; message: string };

/** The Runtime Upload POST result shape: `ok: true` plus the command's fields, or a coded failure. */
export type HostResult<R extends object> = ({ ok: true } & R) | HostFailure;

/** `POST {companion url}command`, Vita to companion. `payload` is omitted for commands that take none. */
export type HostCommandRequest = {
  protocolVersion: number;
  plugin: string;
  command: string;
  payload?: unknown;
};

/** `POST http://{vita}:{port}/link`, companion to Vita. `id` stays the same across relinks of one companion. */
export type HostControlLinkRequest = {
  protocolVersion: number;
  id: string;
  name: string;
  url: string;
  /** Update this id's existing link only; VitaDeck answers 404 `unknown_link` when it has none, e.g. after the user forgot it. */
  relink?: boolean;
};

export function isWireErrorCode(code: unknown): code is HostControlWireErrorCode {
  return typeof code === "string" && Object.hasOwn(HOST_CONTROL_WIRE_ERRORS, code);
}
