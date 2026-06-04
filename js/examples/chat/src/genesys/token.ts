import { getClientCredentialsToken } from "./auth";

export async function getAccessToken(): Promise<string> {
  const token = await getClientCredentialsToken();
  return token.access_token;
}
