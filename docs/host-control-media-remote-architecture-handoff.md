# Handoff: Host Control media remote prototype → architecture improvements

**For:** Architecture agent  
**From:** Cloud agent session (media remote prototype)  
**Date:** 2026-05-26  
**Repo:** `matemolnar8/vitadeck`

---

## Goal for the next session

Use the **media remote prototype** as evidence to recommend and/or implement **architecture and interface-design improvements** for Host Control, Deck Apps, and the dev/agent loop—not to polish the throwaway UI.

The prototype answered “yes, end-to-end works” on Linux; the open work is **how the system should be shaped** before promoting any of this to product.

---

## Context (do not re-read entire PR)

| Artifact | Location |
|----------|----------|
| Draft PR (prototype + `host.media.*` + DX review) | https://github.com/matemolnar8/vitadeck/pull/14 |
| Branch | `cursor/media-remote-prototype-2e84` (base: `host-control-companion`) |
| Prototype Deck App | `js/examples/_prototype-media-remote/` |
| Prototype notes / verdict checklist | `js/examples/_prototype-media-remote/NOTES.md` |
| Host Control iteration-0 spec | `docs/host-control-starter-iteration.html` |
| Domain glossary | `CONTEXT.md` (Host Control Companion, LAN JSON Result, etc.) |
| Command registry | `js/packages/sdk/src/host-control/registry.ts` |
| Companion implementations | `js/packages/host-control-companion/src/implementations.ts`, `media-keys.ts` |
| Reference Deck App (pre-prototype) | `js/examples/host-control/` |

**Branch reality:** `host-control-companion` is **not merged to `main`**. `main` may still contain stale `js/packages/sdk/dist/host-control/` without matching `src/`—a structural hazard for agents and humans.

---

## What was proven

1. Deck App → `createHostControlClient()` → native HTTP → companion → OS action works for a **pad grid** use case.
2. Adding commands requires touching **registry** (contract) + **companion implementations** (platform)—no third layer today.
3. `host.capabilities.commands` is **platform-filtered**; Deck Apps can adapt UI but the prototype did not.
4. Media control required **new** `host.media.*` commands; they were not in the starter iteration doc.

---

## Architecture questions to resolve

Prioritize these; produce ADR candidates or updates to `CONTEXT.md` / planning HTML where appropriate.

### 1. Command model for “remote” features

- **Today:** One HTTP command string per action (`host.media.play_pause`, …).
- **Alternative:** `host.media.action` with payload `{ action: "play_pause" | ... }` to reduce registry churn and companion boilerplate.
- **Trade-off:** Type safety vs. extensibility vs. Stream Deck–style fixed buttons.

### 2. Registry ↔ companion coupling

- Registry lives in `@vitadeck/sdk`; implementations in `@vitadeck/host-control-companion`.
- Adding N media keys meant N registry entries + N implementation entries + shared `media-keys.ts`.
- **Deepen:** Is there a single “command definition” source that generates both typed client API and companion dispatch stubs?

### 3. Platform capability surface

- `availableCommands()` already filters by OS; Deck Apps must interpret `host.capabilities` manually.
- **Proposal area:** SDK-level `useHostCapabilities()` or `isCommandAvailable(cmd)` so UI layers stay thin.

### 4. Client API and dynamic UIs

- `client.command(name)` is strongly typed per literal; **dynamic** command names (pad grids, plugins) do not type-check.
- Prototype used per-pad closures (`run: () => client.command("host.media.play_pause")`).
- **Proposal area:** Documented pattern vs. `commandUnsafe` / plugin command namespace.

### 5. Dev topology vs. product topology

Current desktop loop is fragmented:

- Shell-managed **Host Control Address Setting**
- Separate **companion** process
- **Deck App package** install path (`vitadeck-data/installed-deck-apps/`)
- Native **vitadeck** binary + JS runtime copy

Prototype scripted this in `run-host-prototype.sh`. **Architecture target:** one documented “dev session” abstraction (not necessarily one binary).

### 6. Branch / artifact hygiene

- Unmerged feature branch + committed or stale `dist/` on `main` breaks “source of truth” for agents.
- **Decision:** merge strategy for `host-control-companion`, and policy for `dist/` on default branch.

### 7. OS integration layer

- Prototype uses **throwaway** `xdotool` / `osascript` in `media-keys.ts` (marked PROTOTYPE).
- **Product question:** dbus/MPRIS vs. simulated keys vs. per-app APIs; error model (`command_failed` vs. `unsupported_platform`).

### 8. Documentation as architecture

- `docs/host-control-starter-iteration.html` stops before media/remote.
- Extend planning doc **or** add ADR for `host.media` before merge to avoid each app inventing command names.

---

## Explicit non-goals for architecture agent

- Do not treat `js/examples/_prototype-media-remote/` as production UI.
- Do not expand prototype scope (Spotify API, arbitrary shell, etc.) without threat-model / ADR.
- Human verdict checklist in `NOTES.md` is still empty—hardware UX validation is out of band.

---

## Suggested skills

| Skill | Why |
|-------|-----|
| `improve-codebase-architecture` | Primary: deepening passes, `CONTEXT.md` / ADR alignment, interface design |
| `grill-with-docs` | Stress-test command-model and capability API decisions against glossary |
| `prototype` | Only if a **logic** prototype is needed (e.g. command dispatch state machine)—not more UI |
| `vitadeck-build` | If validating changes across JS + native host build |
| `handoff` | If splitting work again after architecture decisions |

---

## Suggested first actions

1. Read PR #14 description (DX review section)—do not re-derive from chat.
2. Diff `host-control-companion` vs. `main` for Host Control surface area.
3. Run `improve-codebase-architecture` (or equivalent) focused on **Host Control module boundaries** and **Deck App ↔ companion contract**.
4. Output: short list of ADRs / doc updates + optional refactor sketch (registry codegen, dev CLI, capabilities hook).

---

## Sensitive data

None captured in this handoff. Host Control uses **Host Control Trusted LAN** (no tokens in iteration 0).
