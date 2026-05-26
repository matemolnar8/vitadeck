import type { HostControlFailureCode, VitaDeckLanJsonResult } from "@vitadeck/sdk/host-control";

export type HostControlJsonResponse = {
  status: number;
  body: VitaDeckLanJsonResult<object>;
};

export function hostControlFailure(
  status: number,
  code: HostControlFailureCode,
  message: string,
): HostControlJsonResponse {
  return { status, body: { ok: false, code, message } };
}

export function hostControlSuccess<T extends object>(body: T): HostControlJsonResponse {
  return { status: 200, body: { ok: true, ...body } };
}
