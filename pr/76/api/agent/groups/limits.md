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

Review concern 03 of 05: Explicit thread stack and QuickJS limits (`limits`)

The why:

Every Vita thread gets 256 KiB while QuickJS's default guard assumes 1 MiB, and `JS_SetMaxStackSize` / `JS_SetMemoryLimit` were never called ([#53](source:s1)).

The what:

`VD_THREAD_STACK_SIZE` in `thread.h` sets every `vd_thread`'s stack on both platforms, and `js_thread_func` derives the QuickJS stack limit from it minus 64 KiB and caps the JS heap at 64 MiB.

Look for:
- Subtle: the host stack applies to fetch workers and upload handler threads too; on Linux they drop from glibc's 8 MiB default to 1 MiB, on macOS they grow from 512 KiB.
- The 64 MiB heap limit also applies on host, so a Deck App that allocates past it now shows `out of memory` on the Deck App Failure Screen during desktop development, matching the Vita.

Hunk refs for this concern:
- src/platform/thread.h
- src/platform/thread_posix.c
- src/platform/thread_vita.c@4+4
- src/platform/thread_vita.c@31+30
- src/core/js_runtime.c