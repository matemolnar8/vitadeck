import { genesysConfig } from "./config";

export type GenesysTokenResponse = {
  access_token: string;
  token_type: string;
  expires_in: number;
};

function loginBaseUrl(): string {
  return `https://login.${genesysConfig.environment}`;
}

const BASE64_CHARS = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

function base64EncodeAscii(value: string): string {
  const bytes: number[] = [];
  for (let i = 0; i < value.length; i++) {
    bytes.push((value.codePointAt(i) ?? 0) & 0xff);
  }
  let output = "";
  for (let i = 0; i < bytes.length; i += 3) {
    const b0 = bytes[i] ?? 0;
    const b1 = bytes[i + 1];
    const b2 = bytes[i + 2];
    const triplet = (b0 << 16) | ((b1 ?? 0) << 8) | (b2 ?? 0);
    output += BASE64_CHARS[(triplet >> 18) & 63];
    output += BASE64_CHARS[(triplet >> 12) & 63];
    output += b1 === undefined ? "=" : BASE64_CHARS[(triplet >> 6) & 63];
    output += b2 === undefined ? "=" : BASE64_CHARS[triplet & 63];
  }
  return output;
}

function basicAuthHeader(): string {
  const credentials = `${genesysConfig.clientId}:${genesysConfig.clientSecret}`;
  return `Basic ${base64EncodeAscii(credentials)}`;
}

export async function getClientCredentialsToken(): Promise<GenesysTokenResponse> {
  const res = await fetch(`${loginBaseUrl()}/oauth/token`, {
    method: "POST",
    headers: {
      Authorization: basicAuthHeader(),
      "Content-Type": "application/x-www-form-urlencoded",
    },
    body: "grant_type=client_credentials",
  });

  if (!res.ok) {
    throw new Error(`Token request failed (${res.status}): ${await res.text()}`);
  }

  return res.json<GenesysTokenResponse>();
}
