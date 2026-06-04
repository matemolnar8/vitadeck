import { genesysConfig } from "./config";

export function apiBaseUrl(): string {
  return `https://api.${genesysConfig.environment}`;
}

export function genesysApiHeaders(accessToken: string): Record<string, string> {
  const headers: Record<string, string> = {
    Authorization: `Bearer ${accessToken}`,
    "Content-Type": "application/json",
  };
  const genesysApp = genesysConfig.genesysApp;
  if (genesysApp) {
    headers["genesys-app"] = genesysApp;
  }
  return headers;
}

export async function readJsonResponse<T>(res: VitaResponse): Promise<T> {
  if (!res.ok) {
    throw new Error(`HTTP ${res.status}: ${await res.text()}`);
  }
  return res.json<T>();
}
