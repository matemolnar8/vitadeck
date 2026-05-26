# Host Control — Implementation phases

Order assumes connection topology is chosen in phase 0. Adjust if grill resolves a different shape.

## Phase 0 — Design lock (documents + CONTEXT)

- [ ] Resolve connection topology (see [architecture-sketch.md](./architecture-sketch.md))
- [ ] Add glossary terms to [CONTEXT.md](../../CONTEXT.md) (no implementation detail in glossary)
- [ ] ADR only if a decision is hard to reverse (e.g. WebSocket vs HTTP-only)

## Phase 1 — Contract package

- Shared TypeScript module (likely under `js/packages/`) exporting:
  - `protocolVersion`, error codes, request/response types
  - `defineHostControlCommands` + `host.capabilities` + `host.echo`
- Contract tests (parse/validate golden JSON fixtures)
- No Vita native code yet

## Phase 2 — Host companion (TS)

- CLI entry: `vitadeck-host-control start --vita 192.168.x.x:PORT` (flags TBD)
- Implements gateway + command dispatch
- Manual test: curl or small script against companion OR vita listener per topology
- Windows + macOS smoke instructions in package README

## Phase 3 — Vitadeck native + shell

- Listener or client per topology
- Shell surface: show Vita **Host Control** address when listener active (lifecycle TBD — may mirror **Runtime Upload Listener** or always-on when enabled)
- Bridge: async host calls from Deck App JS without blocking render thread
- Settings persistence only if topology requires Vita-side state

## Phase 4 — SDK + author API

- Export typed `hostControl` (name TBD) from `@vitadeck/sdk`
- Example Deck App button that calls `host.echo` / reads capabilities
- Documentation in SDK README

## Phase 5 — Real commands (later)

- `host.app.launch`, `host.media.*`, etc., each with platform implementations
- Out of scope until phases 1–4 are stable

## Testing strategy

| Level | What |
|-------|------|
| Unit | Payload validation, JSON codec, registry |
| Integration | Companion + mock Vita peer, or Vitadeck desktop + companion on loopback |
| Manual | Two machines on LAN, latency feel check |

## Deliberately not ported from `host-control-companion`

- Vita-side **host URL** text editor as the primary pairing model
- Any command implementations beyond capabilities/echo until contract is stable on `main`
