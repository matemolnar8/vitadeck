# deck_bootstrap is a dedup module, not a domain layer

`main.c` and the CI smoke harness both need the same native startup sequence: subsystem init, raylib window, font loading, JS runtime start, deck canvas drawing, and teardown. `src/core/deck_bootstrap.c` exists only to share that code so the smoke test cannot drift from the real app.

It is intentionally **not** a VitaDeck domain concept — no glossary entry in `CONTEXT.md`. Shell navigation, input polling, and runtime restart stay in `main.c`. Smoke-specific waits, tree assertions, and golden screenshots stay in `tests/smoke_harness.c`.

**Considered options:** treat the module as a native "Deck App Host" layer (rejected — overstates its role and invites scope creep); inline duplication in both entry points (rejected — drift risk).

**Consequences:** new shared startup behavior belongs in `deck_bootstrap.c` only when both `main.c` and the smoke harness need it identically; shell or test-only logic stays in its own file. Callers that need non-default raylib window flags pass a `VdDeckBootstrapWindowConfig` with `raylib_config_flags`; `main.c` passes `NULL` for raylib defaults.
