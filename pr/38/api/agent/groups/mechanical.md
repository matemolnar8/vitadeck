Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 1f305ea273a1133774b6a1eda851a5ad99278ad3` and `git rev-parse --verify 282d57c499f34473e74cbd281d12ce11fe79eadc` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 282d57c499f34473e74cbd281d12ce11fe79eadc -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: https://github.com/matemolnar8/vitadeck.git

base (merge-base)  1f305ea273a1133774b6a1eda851a5ad99278ad3

head               282d57c499f34473e74cbd281d12ce11fe79eadc

Named refs at pin: 1f305ea273a1133774b6a1eda851a5ad99278ad3 ... 282d57c

Read the diff:

git diff --find-renames 1f305ea273a1133774b6a1eda851a5ad99278ad3 282d57c499f34473e74cbd281d12ce11fe79eadc

Review concern 08 of 09: Import line reformats (`mechanical`)

Part: oxfmt pass

The why:

oxfmt touched one import line in each of these files.

The what:

One-line import reformats with no symbol change; skip them.

Depends on:
- 07 oxfmt line wrapping (`oxfmt`)

Hunk refs for this concern:
- js/examples/minesweeper/src/App.tsx
- js/examples/smoke/src/App.tsx
- js/packages/runtime/rolldown.config.js