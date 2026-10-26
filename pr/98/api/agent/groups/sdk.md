Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 978568cb5b3b89bc65e656c665ead432947b05c6` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 978568cb5b3b89bc65e656c665ead432947b05c6 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               978568cb5b3b89bc65e656c665ead432947b05c6

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 978568cb5b3b89bc65e656c665ead432947b05c6

Review concern 03 of 04: SDK contract and docs (`sdk`)

The why:

Authors need to leave out a Scroll's main-axis size to get the fill, and the docs must state the new flow and hit rules.

The what:

`VitaScrollProps` gains optional `x`/`y`/`width`/`height`, the reconciler defaults them to 0 for the unchanged native call, and `CONTEXT.md`, the README and the JSDoc describe the flow and hit rules.

Depends on:
- 01 Scroll in the Stack flow (`layout`)
- 02 Hit-transparent Stacks (`hit`)

Hunk refs for this concern:
- js/packages/sdk/src/vitadeck-host-types.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts
- js/packages/sdk/src/index.tsx
- js/packages/sdk/README.md
- CONTEXT.md