Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 37890a509945b80e7dbaeab3a5a87c58234b5e14` and `git rev-parse --verify a724ceea7f72a91e7420366537b01c70afbb9126` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 a724ceea7f72a91e7420366537b01c70afbb9126 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  37890a509945b80e7dbaeab3a5a87c58234b5e14

head               a724ceea7f72a91e7420366537b01c70afbb9126

Named refs at pin: 37890a509945b80e7dbaeab3a5a87c58234b5e14 ... t3/perf-benchmark-suite

Read the diff:

git diff --find-renames 37890a509945b80e7dbaeab3a5a87c58234b5e14 a724ceea7f72a91e7420366537b01c70afbb9126

Review concern 02 of 05: Timing hooks in the UI and JS loops (`hooks`)

Part: Benchmark Mode

The why:

The state machine needs span marks from the render loop and the JS loop, plus commit and input timing from the reconciler.

The what:

`main.c`, `render.c`, `js_runtime.c` and `jslib.c` call the `bench_*` marks around existing work, the reconciler reports commits through `nativeBenchCommit` only when `nativeBenchActive()` was true at load, and `package_library_set_active_transient` lets a scenario switch Deck App without persisting it.

Look for:
- While Benchmark Mode runs, `bench_apply_input` replaces the pad and keyboard polls in `main.c`, so real input is ignored until the run exits.
- `main.c` skips `shell_show_home` in Benchmark Mode, so a run never opens on the VitaDeck Shell.
- `process_input_events` drains the whole event queue before the loop swaps a snapshot; in `minesweeper-navigate` the backlog never clears, so no swap happens in the window and JS iteration stats are null.
- The failure-screen golden changes only in the `runtime.js` line and column of its stack trace, which moved with the bundle.

Depends on:
- 01 Benchmark Mode state machine and clocks (`core`)

Hunk refs for this concern:
- src/main.c
- src/ui/render.c
- src/core/js_runtime.c
- src/jslib/jslib.c
- src/core/event_queue.c
- src/core/event_queue.h
- tests/event_queue_harness.c
- src/ui/instance_tree.c
- src/ui/instance_tree.h
- src/core/package_library.c
- src/core/package_library.h
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@399+400
- tests/fixtures/golden/Linux/13-failure-screen.png
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@384+383