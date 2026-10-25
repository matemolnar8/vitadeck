Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` and `git rev-parse --verify 56d626854fbd0253c445545cf51d4e49db77d014` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 56d626854fbd0253c445545cf51d4e49db77d014 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  5cbf7dacf365fc2a6999c53b263160e1c53db5d8

head               56d626854fbd0253c445545cf51d4e49db77d014

Named refs at pin: origin/perf/dirty-flag-and-caches ... build/warnings-sanitizers-js-limits

Read the diff:

git diff --find-renames 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 56d626854fbd0253c445545cf51d4e49db77d014

Review concern 05 of 05: js_limits harness (`harness`)

The why:

Proves that hitting each limit reaches the Deck App Failure Screen state instead of crashing, on the host and on the Vita stack size.

The what:

`tests/js_limits_harness.c` writes stand-in `js/runtime.js` files and runs them on the real JS thread, and CMake builds it twice, once forcing `VD_THREAD_STACK_SIZE` to the Vita value, on a source list shared with `smoke_harness`.

Look for:
- The fetch cases use `file://` URLs, so they depend on the host libcurl having the file protocol, which Ubuntu and macOS builds do.

Depends on:
- 03 Explicit thread stack and QuickJS limits (`limits`)
- 04 fetch body cap (`fetch-cap`)

Hunk refs for this concern:
- tests/js_limits_harness.c
- CMakeLists.txt@220+242
- CMakeLists.txt@238+260