# Host Control — Overview

## Problem

Deck Apps are rich, touch-oriented UIs on a Vita. Authors and users often want those UIs to control a nearby PC or Mac (launch apps, media transport, shortcuts) with low latency and typed APIs in the **VitaDeck SDK**.

## Goals (this initiative)

1. **Pairing UX** — Vitadeck displays a LAN-reachable address (IP and port). The user types that into the host companion; the **host initiates** setup (not “type your PC’s IP into the Vita”).
2. **Low latency** — Suitable for interactive Deck App UI (subjective target: “feels instant” on a home LAN; exact ms budget TBD).
3. **Host companion** — TypeScript, runnable in the background without a required GUI; Windows and macOS first.
4. **Communication contract first** — Versioned, typed request/response shape; concrete commands (launch, media, etc.) land later behind the contract.
5. **SDK ergonomics** — Deck App authors discover capabilities through TypeScript types (command names, payloads, results).
6. **Alignment with VitaDeck patterns** — Reuse ideas from **Runtime Upload** where they fit (LAN JSON results, listener lifecycle, port fallback), without overloading upload terminology.

## Non-goals (initial slice)

- Internet-wide or account-based pairing
- Linux host support (may follow; not a gate for the first contract)
- Building every command implementation before the wire format exists
- Replacing **Runtime Upload** or merging upload and host-control HTTP servers without an explicit decision

## Success criteria (phase 0 — contract + companion shell)

- Documented wire protocol with `protocolVersion` and machine-oriented error codes
- Host companion: CLI/daemon, binds or connects per chosen topology, implements `host.capabilities` (or equivalent) and a noop/echo command for integration tests
- SDK: typed client surface for registered commands; runtime can call host when linked
- Vitadeck: shows connection info in **VitaDeck Shell** (exact screen TBD); persists whatever the chosen topology requires (may be nothing on Vita if host-only pairing)

## Stakeholders

| Role | Needs |
|------|--------|
| Deck App author | Typed `host.*` (or similar) APIs, clear errors when host unreachable |
| VitaDeck user | Simple pairing: read IP on Vita, paste into host app once |
| Host user | Background service, minimal setup, trustworthy LAN-only behavior |

## Open decisions

See [architecture-sketch.md](./architecture-sketch.md). The first blocking decision is **steady-state connection topology** (who listens, who calls whom for commands).
