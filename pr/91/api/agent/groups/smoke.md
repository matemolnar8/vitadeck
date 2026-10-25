Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2` and `git rev-parse --verify 8468e93ba1ac7e88961ea5137dc35753a0e46643` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2

head               8468e93ba1ac7e88961ea5137dc35753a0e46643

Named refs at pin: 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 ... HEAD

Read the diff:

git diff --find-renames 2f6235eb4fa1d7d7d8f9eecf721c64078ec13df2 8468e93ba1ac7e88961ea5137dc35753a0e46643

Review concern 04 of 05: Smoke golden with colour emoji (`smoke`)

The why:

[Máté's review](source:s2) asks for an emoji in the golden.

The what:

The smoke Button label gains 👍 and the refreshed golden shows it and the header's 🎮 in colour instead of the missing-glyph box.

Depends on:
- 03 Drawing and measuring by cluster (`text`)

Hunk refs for this concern:
- js/examples/smoke/src/App.tsx
- tests/fixtures/smoke_golden.Linux.png