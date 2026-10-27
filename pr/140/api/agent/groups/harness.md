Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify 411554aaf96796d4f990e94021382e9e01662620` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 411554aaf96796d4f990e94021382e9e01662620 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               411554aaf96796d4f990e94021382e9e01662620

Named refs at pin: origin/main ... HEAD

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 411554aaf96796d4f990e94021382e9e01662620

Review concern 02 of 04: The harness and its CTest wiring (`harness`)

The why:

This is the regression check the brief asks for [brief](source:s1); it reproduces the hang of [444f2ce](source:s2) when that fix is reverted.

The what:

`run_deck_app_input_test.sh` stages the examples into a build-dir data root and runs `deck_app_input_harness`, which CMake builds from `HOST_RUNTIME_SOURCES` and registers with the `smoke` label and a 300 s TIMEOUT.

Look for:
- Subtle: `press` waits for the queue to drain after the release, so a handler stuck in `onPressStart` leaves the pressend queued and fails as "JS thread hung after <button>"; a handler stuck in `onPressEnd` is reported against the next button pressed.
- `ROUNDS 12` matches the demo's 12 sections because each round ends L, R, R; adding a demo section without bumping it leaves the last section's face buttons unpressed.

Depends on:
- 01 event_queue_is_empty (`queue-probe`)

Hunk refs for this concern:
- tests/deck_app_input_harness.c
- tests/run_deck_app_input_test.sh
- CMakeLists.txt