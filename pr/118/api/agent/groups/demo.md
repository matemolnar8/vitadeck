Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 21f616e174d3a9beedead96ee8bb6df07b9b2bca` and `git rev-parse --verify 3d85b50f63380f9a26549115361a6f2b959e778c` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 21f616e174d3a9beedead96ee8bb6df07b9b2bca 3d85b50f63380f9a26549115361a6f2b959e778c -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  21f616e174d3a9beedead96ee8bb6df07b9b2bca

head               3d85b50f63380f9a26549115361a6f2b959e778c

Named refs at pin: 21f616e174d3a9beedead96ee8bb6df07b9b2bca ... feat/stack-lays-out-text

Read the diff:

git diff --find-renames 21f616e174d3a9beedead96ee8bb6df07b9b2bca 3d85b50f63380f9a26549115361a6f2b959e778c

Review concern 04 of 05: Demo Label without its wrapper Rect (`demo`)

The why:

The demo was written around the missing Text layout ([brief](source:s3)).

The what:

`Label` in `ui.tsx` renders a bare `Text`, and each section's `lines={n}` becomes `wrap` because the Text now measures its own height.

Look for:
- Every static wrapped label and blurb measures, at the Runtime Default Font, to the line count it used to reserve; Button hints and the Fetch description take their real height instead.

Depends on:
- 02 Text takes its slot in a Stack (`layout`)

Hunk refs for this concern:
- js/examples/demo/src/ui.tsx
- js/examples/demo/src/sections/ButtonSection.tsx
- js/examples/demo/src/sections/FailureSection.tsx
- js/examples/demo/src/sections/FetchSection.tsx
- js/examples/demo/src/sections/PadButtonSection.tsx
- js/examples/demo/src/sections/StackSection.tsx
- js/examples/demo/src/sections/TextSection.tsx
- js/examples/demo/src/sections/TimersSection.tsx
- js/examples/demo/src/sections/StorageSection.tsx
- js/examples/demo/src/sections/TextInputSection.tsx