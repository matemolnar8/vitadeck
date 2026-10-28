import { HOST_CONTROL_PROTOCOL_VERSION, corePlugin } from "@vitadeck/sdk/host-control";
import { implementHostPlugin, type ImplementedHostPlugin } from "./plugin";

export function implementCorePlugin(
  name: string,
  platform: string,
  loadedPlugins: () => readonly ImplementedHostPlugin[],
): ImplementedHostPlugin {
  return implementHostPlugin(corePlugin, {
    capabilities: () => ({
      protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
      name,
      platform,
      plugins: loadedPlugins().map((plugin) => ({ id: plugin.id, commands: [...plugin.commands.keys()] })),
    }),
    echo: ({ text }) => ({ text }),
  });
}
