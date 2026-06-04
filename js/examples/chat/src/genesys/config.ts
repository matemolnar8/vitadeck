import { genesysConfig as local } from "./genesys.local";
import { genesysConfig as defaults } from "./genesys.local.example";

const localNav = "navigationContext" in local && local.navigationContext ? local.navigationContext : undefined;

export const genesysConfig = {
  ...defaults,
  ...local,
  navigationContext: {
    ...defaults.navigationContext,
    ...localNav,
  },
};

export function isGenesysConfigured(): boolean {
  return Boolean(genesysConfig.clientId && genesysConfig.clientSecret && genesysConfig.environment);
}
