# Host Control Companion

Connects from your Mac or PC to Vitadeck’s **LAN HTTP URL**, registers a **Host Callback URL**, and executes **Host Control** commands (`POST /v1/command` on the host).

## Usage

1. Run Vitadeck on the Vita (or desktop). Note the **LAN HTTP URL** on the Shell (e.g. `http://192.168.1.50:8787/`).
2. From the repo:

```bash
pnpm --filter @vitadeck/host-control-companion start -- --vita http://192.168.1.50:8787
```

The companion binds the command listener (default ports **8797–8806**), links to the Vita, then stays running. Deck Apps call the host via the persisted **Host Callback URL**.

**Order matters:** start Vitadeck, then start the companion and wait until you see `linked to Vita` before using **Host echo** in a Deck App. On desktop loopback, pass `--callback-host 127.0.0.1`. On a real Vita, omit that flag so the callback uses your Mac/PC LAN address (the Vita cannot reach `127.0.0.1` on your computer).

### Options

- `--vita URL` — Vitadeck **LAN HTTP URL** (saved to `~/.vitadeck/host-control.json`)
- `--callback-host HOST` — Override the host part of the callback URL (default: LAN IPv4). Use `127.0.0.1` for same-machine desktop dev.

## Hybrid C flow

1. `POST /v1/host/link` on the Vita with `{ protocolVersion, hostName, callbackUrl }`
2. Vitadeck persists **Host Callback URL**
3. Deck Apps → Vita native HTTP client → `POST {callbackUrl}/v1/command`
