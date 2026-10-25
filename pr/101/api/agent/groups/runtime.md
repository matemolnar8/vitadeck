Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5` and `git rev-parse --verify 1ac414061e4d8ecdbe718effe4daca0449592e05` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 1ac414061e4d8ecdbe718effe4daca0449592e05 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5

head               1ac414061e4d8ecdbe718effe4daca0449592e05

Named refs at pin: 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 ... feat/demo-deck-app

Read the diff:

git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 1ac414061e4d8ecdbe718effe4daca0449592e05

Review concern 05 of 06: Runtime API sections (`runtime`)

The why:

The brief asks for Storage, Timers, fetch and the Failure Screen.

The what:

Storage, Timers, fetch and Failure Screen sections use the Deck App globals, and `StorageSection` owns the `demo.counter`/`demo.name` keys the Text Input section imports.

Look for:
- `FetchSection` requests `https://api.github.com/repos/matemolnar8/vitadeck` unauthenticated each time it mounts; GitHub allows 60 such requests per hour per IP, after which the error state shows `HTTP 403`.
- `TimersSection` reads `getTime()` inside the interval callback, not during render, so the React Compiler cannot memoize a stale value.
- Recovery text says to activate another Deck App and then the demo again, because the Shell does not restart a Deck App that is already active.

Depends on:
- 02 App frame and layout helpers (`frame`)

Hunk refs for this concern:
- js/examples/demo/src/sections/StorageSection.tsx
- js/examples/demo/src/sections/TimersSection.tsx
- js/examples/demo/src/sections/FetchSection.tsx
- js/examples/demo/src/sections/FailureSection.tsx