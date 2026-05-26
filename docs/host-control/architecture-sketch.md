# Host Control — Architecture sketch

Status: **draft** — decisions marked **OPEN** are for grill-with-docs.

## Layers

```mermaid
flowchart TB
  subgraph vita [Vita — Vitadeck]
    DA[Deck App JS]
    RT[VitaDeck Runtime Bundle]
    NAT[Native bridge]
    SH[VitaDeck Shell]
    DA --> RT --> NAT
    SH --> NAT
  end

  subgraph lan [LAN]
    W[Wire protocol]
  end

  subgraph host [Host — Companion TS]
    CLI[CLI / background service]
    GW[Protocol gateway]
    CMD[Command implementations]
    CLI --> GW --> CMD
  end

  NAT <-->|OPEN: topology| W
  GW <-->|OPEN: topology| W
```

| Layer | Owner | Responsibility |
|-------|--------|----------------|
| **Deck App** | Author | UI that triggers host actions via **VitaDeck Runtime API** |
| **SDK** | `@vitadeck/sdk` | Command registry, types, client helpers |
| **Native bridge** | Vitadeck C | Network I/O off the JS thread, settings, shell UI |
| **Companion** | New TS package | Listen or connect per topology; execute commands |
| **Contract** | Shared spec | JSON messages, errors, versioning |

## Prior branch (`host-control-companion`) — what to reuse vs drop

| Aspect | Old branch | Fresh direction (from product goals) |
|--------|------------|-------------------------------------|
| Pairing | Vita stores **host** `http://…` URL in settings | User enters **Vita** address in host app; **host initiates** link |
| Steady-state calls | Vita HTTP **client** → host HTTP server | **OPEN** — see below |
| JSON shape | `VitaDeckLanJsonResult`, `{ command, payload? }` | Likely reuse pattern; name terms in CONTEXT.md |
| Ports | Default `8797`, 10-port fallback on **host** | **OPEN** — listener may move to Vita |
| Commands | `host.capabilities`, `host.echo`, stubs | Keep registry pattern; defer implementations |
| Shell | Host URL editor on Vita | Show Vita IP/URL for host; **OPEN** screen design |

## Connection topology — **resolved: Option B**

**Decision:** Vita-as-server; host companion connects using Vita IP shown in Shell. Phase 1: **B-HTTP** (long-poll on shared LAN listener). Phase 2: optional **B-WS** after hardware spike ([vita-http-server-libraries.md](./vita-http-server-libraries.md)).

## Connection topology — options (reference)

Three families of designs:

### A. Vita-as-client (old branch)

- Host runs HTTP server; Vita saves host base URL; each command is `POST` from Vita.
- **Pros:** Host easy to firewall; matches “host executes commands.”
- **Cons:** Opposite of “type Vita IP into host”; Vita must know host address.

### B. Vita-as-server, host-as-client (pairing-aligned)

- Vita runs **Host Control Listener** (like **Runtime Upload Listener**); companion connects using displayed `http://VITA_IP:PORT/…`.
- Commands: **OPEN** — host could poll, or WebSocket for host→Vita pull of queued work, or Vita still POSTs to host on a channel opened during handshake.
- **Pros:** Matches displayed Vita IP; host “dials in.”
- **Cons:** Vita exposes another LAN service; NAT/threading on device.

### C. Hybrid handshake

- Host connects to Vita once (registration); Vita learns host callback URL or socket for subsequent command delivery.
- **Pros:** Flexible latency (persistent channel).
- **Cons:** More state on both sides; harder to explain in CONTEXT.md.

**Lifecycle (resolved):** One **LAN HTTP Listener** (shared port, path routing). Starts at Vitadeck launch, stops at quit—including while a **Deck App** is active and Shell is hidden. No Shell on/off toggle. **LAN HTTP Listener Recovery** (retry on network failure) is **deferred**; first ship may only show bind failure in the **Shell LAN Strip**.

**Shell UX (resolved):** **Shell LAN Strip** on **Shell Home Screen** permanently shows **LAN HTTP URL** + status (listening / bind failure).

**Transport (resolved):** **B-HTTP** long-poll first on the shared listener; optional **B-WS** later. Parse layer: **picohttpparser** (Vita cross-compile confirmed).

## Protocol sketch (command-agnostic)

Aligned with existing **Runtime Upload POST** JSON style:

```json
// Request (direction TBD)
{ "command": "host.capabilities", "payload": {} }

// Success
{ "ok": true, "protocolVersion": 1, "hostName": "…", "commands": ["…"] }

// Failure
{ "ok": false, "code": "unknown_command", "message": "…" }
```

Additional fields likely needed for async/persistent transports: `requestId`, `protocolVersion` on envelope.

## Type-safe SDK shape (resolved)

- **Host Control Contract** lives in **`@vitadeck/sdk`** (dedicated subpath export, e.g. `@vitadeck/sdk/host-control`).
- **Deck Apps** depend only on **`@vitadeck/sdk`** for types and client calls.
- **Host Control Companion** imports the same subpath for registry + gateway types; command OS implementations stay in the companion package.
- Registry pattern: `defineHostControlCommands({ … })`; Deck Apps call typed `hostControl` helpers derived from the registry.

## Security posture (resolved)

- **Host Control LAN Trust** for v1: open LAN, no pairing token (same class as initial **Runtime Upload**). Optional **Upload Pairing**-style hardening later.
- Fail closed on malformed input; size limits on bodies.

## Host companion session (resolved)

- **Host Control Companion** connects to **LAN HTTP URL** at startup (user-configured Vita address).
- **Automatic reconnection** when the session drops while Vitadeck and the listener remain up; user does not re-pair on every disconnect.
- **LAN HTTP Listener Recovery** on the Vita remains deferred; reconnect is companion-driven first.

## Deck App errors (resolved)

- No Vita-side command queue for **Host Control** in v1.
- **Host Control Unavailable** → SDK fails fast with typed error; **Deck App** handles UI.

## Host companion configuration (resolved)

- **Host Control Companion Configuration** persists the Vitadeck **LAN HTTP URL** on the host (e.g. for login-item / background start).
- CLI `--vita` (or equivalent) overrides for one run without changing saved config.

## Background host app

- Node.js CLI with `start` / `stop` or single long-running process.
- No GUI required; optional system tray later.
- Packaging: **OPEN** (npm global, standalone binary via `pkg`/`bun`, or dev-only `pnpm` script first).
