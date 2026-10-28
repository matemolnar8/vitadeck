# VitaDeck Host Companion

The Host Companion runs on a Mac, Windows or Linux computer near your Vita. A Deck App sends it **Host Control** commands over the LAN, and the companion runs them on the computer. It is not published to npm yet; run it from a VitaDeck checkout with Node.js 24 (`pnpm --dir js install` once).

**Security:** v1 trusts the LAN. There are no credentials: a link exchanges addresses and authorizes nothing, and IP checks can be spoofed. That is why the only plugins so far read information. Plugins that act on the computer (keys, launching apps) wait for the credential decision in [ADR 0004](../../../docs/adr/0004-host-control-sliced-delivery.md).

## Use

```sh
pnpm --dir js host-companion start
```

Serves `POST /command` on every interface, port 8797. The port never moves to a free one, because the Vita stores the URL; if it is busy, `start` exits with an error. It prints its URLs, the loaded plugins, and one line per request (time, peer, `plugin.command`, result, duration). Payloads are never logged.

```sh
pnpm --dir js host-companion link 192.168.1.177
```

Open Host Control in the VitaDeck Shell and pass the address it shows (add `:port` if it is not 8798). `link` checks that `start` is answering, then tells VitaDeck where to send commands. One Vita can link several computers, and one computer can link several Vitas.

Options: `--port N` (both commands; use the same one for `link` if you started with it), `--plugin <specifier>` (repeatable, `start` only, in addition to the config), `--advertise <ip>` (`link` only), `--config <path>`.

## Config

`~/.vitadeck/host-companion.json`, created on first use:

- `id`: generated once. VitaDeck keeps one link per id, so relinking replaces the old link.
- `name`: shown on the Vita. Defaults to the hostname.
- `port`: 8797.
- `advertise`: the IP put in the link URL. `null` uses the interface your computer routes to the Vita, which skips VPN and Docker interfaces.
- `vitas`: Vita IPs that `link` added. While it is empty, the companion answers everyone; once it is not, only those Vitas, loopback and this computer's own addresses get answers, and everyone else gets `forbidden`.
- `plugins`: plugins to load next to the built-in `core` plugin. Defaults to `@vitadeck/host-plugin-system-stats`.

## Plugins

A **Host Control Plugin** is a package with two exports:

- `.`: the registry a Deck App imports and passes to `createHostClient`. It is bundled into the Deck App and runs on QuickJS, so it imports only `@vitadeck/sdk/host-control`:

  ```ts
  export const lightsPlugin = defineHostPlugin("lights", {
    toggle: command<{ room: string }, { on: boolean }>(isRoom),
  });
  ```

- `./companion`: the implementation, which default-exports `implementHostPlugin(registry, implementations)` from `@vitadeck/host-companion`. A missing or mistyped implementation fails `tsc`. Throw `new HostCommandError("unsupported_platform", "...")` to answer with a specific code; any other throw answers `command_failed` with its message.

Since `@vitadeck/host-companion` is not published, plugins live in this workspace under `js/host-plugins/`, with the companion as a `peerDependency`. [`system-stats`](../../host-plugins/system-stats) is the example. To load one, either add it to this package's `dependencies` and put its name in `plugins`, or give a path to its implementation file (relative to where you run the command): `--plugin ./js/host-plugins/lights/src/companion.ts`.

## Run in the background

The companion has no installer or tray icon; let the OS start it at login. Replace `/path/to/vitadeck` with your checkout.

**macOS:** save as `~/Library/LaunchAgents/dev.vitadeck.host-companion.plist`, then `launchctl load ~/Library/LaunchAgents/dev.vitadeck.host-companion.plist`. The login shell finds `pnpm` the same way your terminal does.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key><string>dev.vitadeck.host-companion</string>
  <key>ProgramArguments</key>
  <array>
    <string>/bin/zsh</string><string>-lc</string>
    <string>pnpm --dir /path/to/vitadeck/js host-companion start</string>
  </array>
  <key>RunAtLoad</key><true/>
  <key>KeepAlive</key><true/>
  <key>StandardOutPath</key><string>/tmp/vitadeck-host-companion.log</string>
  <key>StandardErrorPath</key><string>/tmp/vitadeck-host-companion.log</string>
</dict>
</plist>
```

**Windows:** create a scheduled task that starts at log on:

```bat
schtasks /Create /SC ONLOGON /TN "VitaDeck Host Companion" /TR "cmd /c pnpm --dir C:\path\to\vitadeck\js host-companion start"
```

**Linux:** save as `~/.config/systemd/user/vitadeck-host-companion.service`, then `systemctl --user enable --now vitadeck-host-companion`.

```ini
[Unit]
Description=VitaDeck Host Companion

[Service]
ExecStart=/bin/bash -lc "pnpm --dir /path/to/vitadeck/js host-companion start"
Restart=on-failure

[Install]
WantedBy=default.target
```

## Firewall

The Vita connects to the companion, so the computer must accept incoming connections on the port. The first `start` triggers a prompt: on macOS, allow `node` to accept incoming network connections; on Windows, allow Node.js on private networks. On Linux, open the port in your firewall if one is running (`sudo ufw allow 8797/tcp`).
