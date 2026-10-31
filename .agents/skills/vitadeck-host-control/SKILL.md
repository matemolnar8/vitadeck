---
name: vitadeck-host-control
description: Connects Deck Apps to a nearby computer with VitaDeck Host Control. Runs and links the Host Companion (vitadeck-host-companion), installs .vdplugin packages, and writes Host Control Plugins with a Deck App or standalone. Use when a Deck App should read or control a Mac, Windows or Linux computer, or when the user mentions Host Control, the Host Companion, vitadeck-host-companion, host plugins, .vdplugin, systemStats, linking a computer, or errors like not_linked, unknown_plugin, unreachable or forbidden.
---

# VitaDeck Host Control

A Deck App calls typed commands on a linked computer. The computer runs the **Host Companion**, which serves the commands of its **Host Control Plugins**:

- **Built-ins** ship in every companion: `core` (`capabilities`, `echo`, `events`, `notify`) and `systemStats` (`read`: CPU, memory, uptime).
- **Custom plugins** are `.vdplugin` packages an author builds with the SDK and the user installs on the computer.

The Vita calls the computer over the LAN. There are no credentials yet, so treat any plugin that acts on the computer as reachable by anyone on the network.

## Run and link a computer

1. Install the companion with Node.js 24: `npm install -g @vitadeck/host-companion`. It is not on npm yet; from a VitaDeck checkout, `pnpm --dir js host-companion <command>` runs the same CLI.
2. Start it with `vitadeck-host-companion start`, which serves port 8797, and allow incoming connections at the firewall prompt.
3. On VitaDeck, press Start (F1 on the desktop host) and note the address under **Upload and link**. VitaDeck listens from startup on port 8787, for uploads and links alike.
4. On the computer, run `vitadeck-host-companion link <address>`. Link more computers the same way; VitaDeck keeps up to 4. **Host Control** in the Shell lists them and forgets one on Cross.

Config, auto-start recipes and firewall notes: [the companion README](https://github.com/matemolnar8/vitadeck/blob/main/js/packages/host-companion/README.md).

## Call it from a Deck App

```tsx
import { createHostClient, linkedHosts, systemStatsPlugin } from "@vitadeck/sdk/host-control";

const stats = await createHostClient(systemStatsPlugin, { hostId: linkedHosts()[0]?.id }).read();
if (stats.ok) setCpu(stats.cpuPercent);
else setHint(`${stats.code}: ${stats.message}`);
```

- Calls never reject. Always branch on `ok`.
- Leave out `hostId` to call the most recent link.
- For things that happen on the computer (an alert, a build finishing), watch events instead of polling: `useEffect(() => watchHostEvents(plugin, (event) => ..., { hostId }), [hostId])`. Each event arrives within about one round trip. `core`'s `notice` events come from `vitadeck-host-companion notify "text"`, so any script can reach Deck Apps.
- When you do poll, go no faster than once a second. After a `timeout`, back off; the demo uses 30 s. See [`js/examples/host-control`](https://github.com/matemolnar8/vitadeck/tree/main/js/examples/host-control).

## Write a plugin

| Where it lives | Scaffold | Built by `vitadeck build` |
| --- | --- | --- |
| Inside a Deck App | `npx @vitadeck/sdk create my-app --host-plugin` | `dist/my-app.vdapp(.zip)` and `dist/my-app.vdplugin(.zip)` |
| On its own, for any Deck App | `npx @vitadeck/sdk create lights --host-plugin-only` | `dist/lights.vdplugin(.zip)` only |

- **`host/plugin.ts`** declares the commands with `defineHostPlugin(id, { name: command<Payload, Result>(guard) })`. Deck Apps bundle it, so it must not import Node.js modules. The id defaults to the project slug.
- **`host/companion.ts`** default-exports `implementHostPlugin(plugin, { name: (payload) => result })` and runs in the companion with Node.js. `tsc -p host` fails on a missing or mistyped command. Throw `new HostCommandError("unsupported_platform", "...")` to answer with a specific code.
- **Events:** declare them in `defineHostPlugin(id, commands, { name: event<Data>() })` and emit them from `implementHostPlugin(plugin, impls, { start: (emit) => { ...; return stop; } })`. `start` runs while the plugin is loaded, including after each reload.
- **`vitadeck.config.json`** holds `"hostPlugin": { "entry": "./host/companion.ts", "files": "./host/files" }`.
  - `files` is optional. Read those files with `new URL("./files/x", import.meta.url)` when a command runs, not at module load.
  - The zip drops file modes, so run a shipped script through its interpreter (`execFile("sh", [path])`).
- **A standalone plugin's `package.json`** exports `host/plugin.ts` and takes `@vitadeck/sdk` as a peer dependency. A Deck App adds it as a dependency (`pnpm add ../lights`, a git URL, or npm once `"private": true` is removed), then runs `import { hostPlugin as lights } from "lights"`.

The SDK README section [Your own Host Plugin](https://github.com/matemolnar8/vitadeck/blob/main/js/packages/sdk/README.md#your-own-host-plugin) has the full reference.

## Develop

1. In one terminal, run `pnpm host` (`vitadeck build && vitadeck-host-companion start --plugin ./dist`).
2. In another, run `vitadeck dev <runtime-upload-url>`. A plugin-only project runs `vitadeck dev` with no URL.
3. Link the companion once (see above). Every save rebuilds the plugin, and the companion prints `Reloaded plugins` with nothing restarted. A broken build prints `Kept the previous plugins` and keeps the last good package.

## Ship and install

- **Author:** ship `dist/<slug>.vdplugin.zip`, next to the `.vdapp.zip` if there is one.
- **User:** run `vitadeck-host-companion install <slug>.vdplugin.zip`. It lands in `~/.vitadeck/plugins/<id>.vdplugin`, and a running companion loads it at once. `vitadeck-host-companion remove <id>` uninstalls it.
- Only install plugins you trust: they run with the user's permissions.

## Troubleshooting

| Code or message | Meaning |
| --- | --- |
| `not_linked` | No computer is linked, or `hostId` was forgotten in the Shell. Link again. |
| `unreachable`, `timeout` | The companion is not running, the firewall blocks 8797, or the computer's IP changed (link again). |
| `unknown_plugin` | That computer has not installed the plugin. Tell the user which `.vdplugin.zip` to install. |
| `forbidden` | The Vita's IP is not the one the companion linked. Link again from the Shell. |
| `invalid_payload` | The payload failed the command's guard. |
| `command_failed` | The implementation threw. The companion logs one line per request. |
| No events arrive | The watcher only gets events after it started; check the companion log for `core.events` lines, which repeat every 25 s while it waits. |
| `Port 8797 is already in use` | Another companion is running. Stop it or pass the same `--port` to `start` and `link`. |
| `--plugin takes a .vdplugin folder` | Point `--plugin` at `dist` or a `.vdplugin` folder; `install` the `.zip`. |

Design and reasoning: [ADR 0004](https://github.com/matemolnar8/vitadeck/blob/main/docs/adr/0004-host-control-sliced-delivery.md).
