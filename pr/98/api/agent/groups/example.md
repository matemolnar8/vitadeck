Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5` and `git rev-parse --verify 76eef5b1bd42f8b9e1039d96814122116bccd798` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 76eef5b1bd42f8b9e1039d96814122116bccd798 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5

head               76eef5b1bd42f8b9e1039d96814122116bccd798

Named refs at pin: 18cbdfd ... feat/scroll-in-stack

Read the diff:

git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 76eef5b1bd42f8b9e1039d96814122116bccd798

Review concern 04 of 04: Timers example (`example`)

The why:

The task asks for the header-plus-scrolling-list pattern in one example.

The what:

The timers app becomes a sized column whose three demo rows sit above a `Scroll` that fills the rest with a tick log derived from `ticks`.

Depends on:
- 03 SDK contract and docs (`sdk`)

Hunk refs for this concern:
- js/examples/timers/src/App.tsx