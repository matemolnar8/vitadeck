# Host Control

Vitadeck **Deck Apps** ask a computer on the same LAN to run actions (launch apps, media keys, shell commands, and similar). The **Host Control Companion** on the PC/Mac links to the Vita **LAN HTTP URL**, registers a **Host Callback URL**, and serves `POST /v1/command`.

## Documents

| Document | Purpose |
|----------|---------|
| [overview.md](./overview.md) | Goals, non-goals, success criteria |
| [architecture-sketch.md](./architecture-sketch.md) | Layering and resolved Hybrid C decisions |
| [implementation-phases.md](./implementation-phases.md) | Delivery phases and checklist |
| [topology-comparison.md](./topology-comparison.md) | Historical A / B / C analysis (shipped: **Hybrid C**) |
| [picohttpparser-vita-spike.md](./picohttpparser-vita-spike.md) | picohttpparser Vita cross-compile spike |

## Related domain language

Terms live in [CONTEXT.md](../../CONTEXT.md).

## Prior art in-repo

- **Runtime Upload** — Vita listens on LAN; browser is the client.
- **`host-control-companion` branch** — Vita-as-client only (host URL in Vita settings). Superseded by Hybrid C (link on Vita, commands to host).
