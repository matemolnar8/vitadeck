Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify c45708bc4894acd44d9184265a1624cda104eeb5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 c45708bc4894acd44d9184265a1624cda104eeb5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               c45708bc4894acd44d9184265a1624cda104eeb5

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 c45708bc4894acd44d9184265a1624cda104eeb5

Review concern 03 of 05: Examples keep their Text offset (`text-call-sites`)

Part: Text defaults

The why:

The examples and scaffold relied on the old 8px offset; passing it explicitly keeps their look while the default moves.

The what:

Each `Text` without `x`/`y` in minesweeper, timers and the scaffold template gains `x={8} y={8}`; smoke already passes every prop, so its golden does not move for this part.

Depends on:
- 02 Text defaults (`text-defaults`)

Hunk refs for this concern:
- js/examples/minesweeper/src/App.tsx
- js/examples/timers/src/App.tsx
- js/packages/sdk/cli/templates/scaffold/src/App.tsx