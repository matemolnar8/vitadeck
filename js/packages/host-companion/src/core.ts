import {
  HOST_CONTROL_PROTOCOL_VERSION,
  corePlugin,
  implementHostPlugin,
  type ImplementedHostPlugin,
} from "@vitadeck/sdk/host-control";
import type { EventLog } from "./events";

export function implementCorePlugin(
  name: string,
  platform: string,
  loadedPlugins: () => readonly ImplementedHostPlugin[],
  events: EventLog,
): ImplementedHostPlugin {
  return implementHostPlugin(corePlugin, {
    capabilities: () => ({
      protocolVersion: HOST_CONTROL_PROTOCOL_VERSION,
      name,
      platform,
      plugins: loadedPlugins().map((plugin) => ({ id: plugin.id, commands: [...plugin.commands.keys()] })),
    }),
    echo: ({ text }) => ({ text }),
    events: ({ plugin, after }) => events.wait(plugin, after),
    notify: ({ text }) => {
      events.emit(corePlugin.id, "notice", { text });
      return {};
    },
  });
}
