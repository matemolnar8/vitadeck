# Host Control

A Deck App for **Host Control**. It lists the computers linked in the VitaDeck Shell with each one's live CPU load, and for the selected one shows its capabilities, CPU and memory gauges from the built-in `systemStats` plugin, and an Echo round trip.

From this repo, `pnpm --dir js host-companion` runs the [Host Companion](../../packages/host-companion/README.md) that `vitadeck-host-companion` installs:

1. On the computer: `pnpm --dir js host-companion start`.
2. On VitaDeck: press Start, open Host Control, and note the link address.
3. On the computer: `pnpm --dir js host-companion link <link address>`.

Link more computers the same way; each gets its own row. D-pad and Cross pick one.
