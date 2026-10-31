# VitaDeck Host Companion

The Host Companion runs on a Mac, Windows or Linux computer near your Vita. A Deck App sends it **Host Control** commands over the LAN, and the companion runs them on the computer. It needs Node.js 24.

```sh
npm install -g @vitadeck/host-companion
```

It is not published to npm yet. From a VitaDeck checkout, `pnpm --dir js host-companion <command>` runs the same CLI.

**Security:** v1 trusts the LAN. There are no credentials: a link exchanges addresses and authorizes nothing, and IP checks can be spoofed. That is why the built-in plugins only read information. Plugins that act on the computer (keys, launching apps) wait for the credential decision in [ADR 0004](../../../docs/adr/0004-host-control-sliced-delivery.md).

## Use

```sh
vitadeck-host-companion start
```

Serves `POST /command` on every interface, port 8797. The port never moves to a free one, because the Vita stores the URL; if it is busy, `start` exits with an error. It prints its URLs, the loaded plugins, and one line per request (time, peer, `plugin.command`, result, duration). Payloads are never logged.

```sh
vitadeck-host-companion link 192.168.1.177
```

Pass the Vita's address: VitaDeck listens from startup, and the VitaDeck Shell shows the address under Upload and link (add `:port` if it is not 8787). `link` checks that `start` is answering, then tells VitaDeck where to send commands. One Vita can link several computers, and one computer can link several Vitas.

Each `start` relinks every Vita this computer linked, so a new IP or `--port` reaches them without running `link` again. A Vita that is off is retried every 30 s; one that forgot this computer in the Shell is dropped from the config instead of being linked again. A Vita whose Upload and link listener moved off port 8787 needs `link` again.

```sh
vitadeck-host-companion notify "Build done"
```

Sends a notice to every Deck App watching this computer's `core` events (the `host-control` demo shows it), for example at the end of a script: `make && vitadeck-host-companion notify "Build done"`. Deck Apps keep one request waiting at the companion for events, renewed every 25 s while nothing happens, so the request log shows a `core.events` line about every 25 s per watching Deck App.

Options: `--port N` (`start`, `link` and `notify`; use the same one for all), `--plugin <name.vdplugin or folder>` (repeatable, `start` only; a folder loads every `.vdplugin` package in it), `--advertise <ip>` (`link` only), `--config <path>`.

## Plugins

Every companion has the built-in plugins: `core` (`capabilities`, `echo`) and `systemStats` (`read`: CPU, memory, uptime). A Deck App imports their registries from `@vitadeck/sdk/host-control`.

A Deck App that needs more brings its own plugin as a **Host Plugin Package**, which its author ships as a `.vdplugin.zip` next to the `.vdapp.zip`:

```sh
vitadeck-host-companion install lights.vdplugin.zip
vitadeck-host-companion remove lights
```

`install` unpacks the archive (or copies a `.vdplugin` folder) into `~/.vitadeck/plugins/`, named by the plugin's id, and a running companion loads it at once. A plugin runs with your permissions on this computer, so install only plugins you trust.

To write one, scaffold a Deck App with `npx @vitadeck/sdk create my-app --host-plugin`, or a plugin on its own with `create my-plugin --host-plugin-only`; the [SDK README](../sdk/README.md#your-own-host-plugin) explains the files. While you work on it, `start --plugin ./dist` loads your build, takes precedence over an installed copy, and reloads every time `vitadeck build` or `vitadeck dev` writes it. A broken build keeps the previous version running.

## Config

`~/.vitadeck/host-companion.json`, created on first use:

- `id`: generated once. VitaDeck keeps one link per id, so relinking replaces the old link.
- `name`: shown on the Vita. Defaults to the hostname.
- `port`: 8797.
- `advertise`: the IP put in the link URL. `null` uses the interface your computer routes to the Vita, which skips VPN and Docker interfaces.
- `vitas`: Vita IPs that `link` added. While it is empty, the companion answers everyone; once it is not, only those Vitas, loopback and this computer's own addresses get answers, and everyone else gets `forbidden`.

Installed plugins live in `plugins/` next to the config file, so `--config` gives a second companion its own plugins too.

## Run in the background

The companion has no tray icon; let the OS start it at login.

**macOS:** save as `~/Library/LaunchAgents/dev.vitadeck.host-companion.plist`, then `launchctl load ~/Library/LaunchAgents/dev.vitadeck.host-companion.plist`. The login shell finds `vitadeck-host-companion` the same way your terminal does.

```xml
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
  <key>Label</key><string>dev.vitadeck.host-companion</string>
  <key>ProgramArguments</key>
  <array>
    <string>/bin/zsh</string><string>-lc</string>
    <string>vitadeck-host-companion start</string>
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
schtasks /Create /SC ONLOGON /TN "VitaDeck Host Companion" /TR "cmd /c vitadeck-host-companion start"
```

**Linux:** save as `~/.config/systemd/user/vitadeck-host-companion.service`, then `systemctl --user enable --now vitadeck-host-companion`.

```ini
[Unit]
Description=VitaDeck Host Companion

[Service]
ExecStart=/bin/bash -lc "vitadeck-host-companion start"
Restart=on-failure

[Install]
WantedBy=default.target
```

## Firewall

The Vita connects to the companion, so the computer must accept incoming connections on the port. The first `start` triggers a prompt: on macOS, allow `node` to accept incoming network connections; on Windows, allow Node.js on private networks. On Linux, open the port in your firewall if one is running (`sudo ufw allow 8797/tcp`).
