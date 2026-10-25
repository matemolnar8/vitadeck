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

Review concern 05 of 05: Glossary, SDK README, setup and build skill (`docs`)

The why:

AGENTS.md asks for the glossary and SDK README to stay accurate when author-visible behaviour changes.

The what:

The **Deck App Font** entry says emoji draw from Twemoji or fall back, the SDK README credits Twemoji, and `AGENTS.md` Setup and the `vitadeck-build` skill say configure downloads the images into the gitignored folder.

Depends on:
- 03 Drawing and measuring by cluster (`text`)

Hunk refs for this concern:
- CONTEXT.md
- js/packages/sdk/README.md
- AGENTS.md
- .agents/skills/vitadeck-build/SKILL.md