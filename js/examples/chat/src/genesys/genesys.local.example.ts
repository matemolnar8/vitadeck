/**
 * Template for local credentials. Copied to `genesys.local.ts` on build/tsc if missing.
 * `genesys.local.ts` is gitignored — put real secrets there only.
 */
export const genesysConfig = {
  /** e.g. inindca.com, mypurecloud.com — used for login.{env} and api.{env} */
  environment: "inindca.com",
  clientId: "",
  clientSecret: "",
  genesysApp: "copilot_framework_webui/main/287",
  preferredLanguage: "en-us",
  timezone: "Europe/Budapest",
  navigationContext: {
    route: "vitadeck/chat",
    pageTitle: "VitaDeck Chat Example",
  },
} as const;
