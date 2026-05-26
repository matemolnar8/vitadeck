# Host Control Companion

Connects from your Mac or PC to Vitadeck’s **LAN HTTP URL** and executes **Host Control** commands (launch apps, media, etc.) via long-poll.

## Setup

1. Run Vitadeck on the Vita (or desktop). Note the **LAN** URL on the Shell (e.g. `http://192.168.1.50:8787/`).
2. Start the companion:

```sh
pnpm --filter @vitadeck/host-control-companion start -- --vita http://192.168.1.50:8787
```

The URL is saved to `~/.vitadeck/host-control.json`. Later runs:

```sh
pnpm --filter @vitadeck/host-control-companion start
```

## Deck Apps

Use `hostControl` from `@vitadeck/sdk` (requires the companion to be connected).
