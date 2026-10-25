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

Review concern 01 of 04: Failure state on VdJsRuntime (`failure-state`)

The why:

The JS thread, the error boundary, and the font loader need one place to record a Deck App failure that the main thread can paint.

The what:

`js_runtime_fail` stores the first message and stack on `VdJsRuntime`, the JS loop exits once `failed` is set, and `clear_failure` runs on init, start, and stop.

Look for:
- Subtle: the JS loop now stops on the first failure, so after a timer throws no more timers, input, or fetch callbacks run; the thread exits and frees the context while `runtime->thread` stays set until `js_runtime_stop` joins it.
- Race: the JS thread writes `failure_message` before setting the `volatile bool failed`, with no memory barrier; this matches the existing `ready` flag protocol and the strings are never written again once `failed` is true.
- `js_runtime_fail` is `static inline` in the header so `timer_reload_harness`, which links only `timeout.c` and `error.c`, does not pull in the runtime thread.

Hunk refs for this concern:
- src/core/js_runtime.h
- src/core/js_runtime.c