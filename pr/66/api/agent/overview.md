Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify d1a60387997054feecfb96ad2f518d0f9cee05f0` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
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

Commits:
- d1a6038 fix(runtime): show a failure screen when a Deck App throws after boot

Sources:
- ticket #44 Add a runtime-owned error boundary and route uncaught JS errors to a native error screen with message, stack excerpt, and Start hint.
  https://github.com/matemolnar8/vitadeck/issues/44

The title:

Show a failure screen when a Deck App throws after boot

The why:

Errors after boot are only logged, React unmounts the root, and [the user sees nothing](source:s1).

The what (medium):

Every uncaught Deck App error (render, effects, input handlers, timers, microtasks, module eval) now records a failure on `VdJsRuntime`, stops the JS loop, and paints the Deck App Failure Screen with the message, a stack excerpt, and the Start hint.

Look for:
- Unhandled promise rejections still only fail silently: QuickJS turns a throw inside a `.then` callback into a rejected promise, so `JS_ExecutePendingJob` does not return an error and `drain_microtasks` never reports it. Catching those needs `JS_SetHostPromiseRejectionTracker`, which this change does not add.

## Review concerns

### 01 Failure state on VdJsRuntime (`failure-state`)

`js_runtime_fail` stores the first message and stack on `VdJsRuntime`, the JS loop exits once `failed` is set, and `clear_failure` runs on init, start, and stop.

[groups/failure-state.md](groups/failure-state.md)

### 02 Native exception reporters (`reporters`)

`js_report_exception` in the new `error.c` replaces the log-only blocks in input dispatch, `run_timeouts`, `drain_microtasks`, `runtime.js` eval, and `updateContainer`, and finds the runtime through `JS_GetContextOpaque`.

Depends on:
- 01 Failure state on VdJsRuntime (`failure-state`)

[groups/reporters.md](groups/reporters.md)

### 03 Runtime error boundary (`boundary`)

`DeckAppErrorBoundary` wraps `ThemeProvider` and the Deck App, renders `null` after an error, and reports it through the new `nativeReportDeckAppError` global.

Depends on:
- 02 Native exception reporters (`reporters`)

[groups/boundary.md](groups/boundary.md)

### 04 Deck App Failure Screen paint (`paint`)

`bootstrap_draw_deck_canvas` replaces the old "failed to start" text with `draw_deck_app_failure_screen`, and `main.c` drops its separate recovery hint and stops polling Deck App input while failed.

Depends on:
- 01 Failure state on VdJsRuntime (`failure-state`)

[groups/paint.md](groups/paint.md)