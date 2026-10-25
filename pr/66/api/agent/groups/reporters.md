Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify d1a60387997054feecfb96ad2f518d0f9cee05f0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d d1a60387997054feecfb96ad2f518d0f9cee05f0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               d1a60387997054feecfb96ad2f518d0f9cee05f0

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... fix/deck-app-error-screen

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d d1a60387997054feecfb96ad2f518d0f9cee05f0

Review concern 02 of 04: Native exception reporters (`reporters`)

The why:

Handlers, timers, effects, and microtasks were [only `TraceLog`ged](source:s1).

The what:

`js_report_exception` in the new `error.c` replaces the log-only blocks in input dispatch, `run_timeouts`, `drain_microtasks`, `runtime.js` eval, and `updateContainer`, and finds the runtime through `JS_GetContextOpaque`.

Look for:
- `timeout.c` changes only the exception branch, so it rebases cleanly onto PR #61's `run_timeouts` rewrite; the old per-tick id log line is gone.
- A context with no opaque runtime (the timer harness) only logs, so `js_report_exception` is safe outside `js_thread_func`.

Depends on:
- 01 Failure state on VdJsRuntime (`failure-state`)

Hunk refs for this concern:
- src/jslib/error.c
- src/jslib/jslib.c
- src/jslib/jslib.h
- src/jslib/jslib_internal.h
- src/jslib/timeout.c
- CMakeLists.txt