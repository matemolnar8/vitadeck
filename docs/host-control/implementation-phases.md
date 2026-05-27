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
- CLI: `start` binds command listener, **Host Control Link** with retry, `--vita` + `--callback-host`
- Implements gateway + `POST /v1/command` server
- Windows + macOS smoke instructions in package README

## Phase 3 — Vitadeck native + shell

- [x] Shared listener routes `/v1/host/link` and `/v1/host/status` (Hybrid C)
- [x] Vita HTTP client (`nativeHostControlFetch`) for `POST /v1/command` on **Host Callback URL**
- [x] Persist **Host Callback URL** (`host-control-link.json` under package library root)
- [x] Always-on listener from `shell_init`
- [x] **Shell LAN Strip**
- [x] Native `nativeHostControlFetch` + `nativeGetHostControlBaseUrl` bridge
- [x] picohttpparser integrated into request parsing (`src/net/http_parse.c`)
- [x] **Shell LAN Strip** shows link status via **Host Control Status**

## Phase 4 — SDK + author API

- [x] Export typed `hostControl` from `@vitadeck/sdk`
- [x] Example Deck App button that calls `host.echo` (`js/examples/default`)
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
