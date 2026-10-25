Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` and `git rev-parse --verify 2d9850830f3b8f421e81f7d6750564e89db51ccc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

head               2d9850830f3b8f421e81f7d6750564e89db51ccc

Named refs at pin: origin/feat/focus-and-pad-input ... feat/text-input

Read the diff:

git diff --find-renames b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 2d9850830f3b8f421e81f7d6750564e89db51ccc

Review concern 06 of 07: Chat compose row (`example`)

The why:

An example Deck App exercises the component without touching the Genesys files.

The what:

The chat compose row adds a `TextInput` whose trimmed draft `sendMessage` sends instead of the scripted suggestion, then clears.

Depends on:
- 05 SDK TextInput and runtime wiring (`js`)

Hunk refs for this concern:
- js/examples/chat/src/App.tsx