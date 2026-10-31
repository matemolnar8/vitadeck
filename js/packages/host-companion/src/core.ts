import {
  HOST_CONTROL_PROTOCOL_VERSION,
  HostCommandError,
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
    events: ({ plugin, after }) => {
      if (plugin !== corePlugin.id && !loadedPlugins().some((loaded) => loaded.id === plugin)) {
        throw new HostCommandError("unknown_plugin", `This Host Companion has no plugin ${plugin}.`);
      }
      return events.wait(plugin, after);
    },
    notify: ({ text }) => {
      events.emit(corePlugin.id, "notice", { text });
      return {};
    },
  });
}
