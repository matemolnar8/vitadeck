# Host Control — Implementation phases

Order assumes connection topology is chosen in phase 0. Adjust if grill resolves a different shape.

## Phase 0 — Design lock (documents + CONTEXT)

- [x] Resolve connection topology (see [architecture-sketch.md](./architecture-sketch.md))
- [x] Add glossary terms to [CONTEXT.md](../../CONTEXT.md)
- [ ] ADR only if a decision is hard to reverse (e.g. WebSocket vs HTTP-only)

## Phase 1 — Contract in SDK

- [x] **`@vitadeck/sdk/host-control`** subpath exporting:
  - `protocolVersion`, error codes, request/response types
  - `defineHostControlCommands` + `host.capabilities` + `host.echo`
- Contract tests (parse/validate golden JSON fixtures)
- No Vita native code yet

## Phase 2 — Host companion (TS)

- [x] In-repo package `js/packages/host-control-companion` (workspace only, not npm publish v1)
- Depends on `@vitadeck/sdk/host-control` subpath
- CLI: `start` with **Host Control Companion Configuration** + `--vita` override
- Implements gateway + command dispatch + auto-reconnect
- Windows + macOS smoke instructions in package README

## Phase 3 — Vitadeck native + shell

- [x] Shared listener routes `/v1/host/poll` and `/v1/host/result`
- [x] Always-on listener from `shell_init`
- [x] **Shell LAN Strip**
- [x] Native `nativeHostControlCommand` bridge
- [ ] picohttpparser integrated into request parsing (vendored; wire-up optional follow-up)
- Shell surface: show Vita **Host Control** address when listener active (lifecycle TBD — may mirror **Runtime Upload Listener** or always-on when enabled)
- Bridge: async host calls from Deck App JS without blocking render thread
- Settings persistence only if topology requires Vita-side state

## Phase 4 — SDK + author API

- [x] Export typed `hostControl` from `@vitadeck/sdk`
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
