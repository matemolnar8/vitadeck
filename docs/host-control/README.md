# Host Control

Vitadeck **Deck Apps** can ask a computer on the same LAN to run actions (launch apps, media keys, shell commands, and similar). The computer runs a small **Host Control Companion** (TypeScript, Windows and macOS at minimum). Vitadeck on the Vita shows connection information; the author enters the Vita address in the companion to link the two sides.

This folder holds planning material for a **fresh** implementation. Prior work on branch `host-control-companion` is reference only — it is not the target design.

## Documents

| Document | Purpose |
|----------|---------|
| [overview.md](./overview.md) | Goals, non-goals, glossary hooks, success criteria |
| [architecture-sketch.md](./architecture-sketch.md) | Layering, open decisions, options vs `host-control-companion` |
| [implementation-phases.md](./implementation-phases.md) | Suggested delivery order (contract → companion → Vita → SDK) |

## Related domain language

Terms are added to [CONTEXT.md](../../CONTEXT.md) as the design is resolved (grill-with-docs). Nothing here replaces the glossary.

## Prior art in-repo

- **Runtime Upload** — Vita listens on LAN (`Runtime Upload Listener`), browser is the client. Similar trust model (LAN, no account), different payload.
- **`host-control-companion` branch** — Vita stored the **host** URL in settings; **Deck Apps** called the host over HTTP. Connection initiation was Vita → host, not host → Vita.
