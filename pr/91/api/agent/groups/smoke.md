Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b6e820df59a1e9576e5d45a9543fa6689edfb6d6` and `git rev-parse --verify e6f928144dc6343cb90c4b20f7c6ecbb247fab82` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b6e820df59a1e9576e5d45a9543fa6689edfb6d6 e6f928144dc6343cb90c4b20f7c6ecbb247fab82 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b6e820df59a1e9576e5d45a9543fa6689edfb6d6

head               e6f928144dc6343cb90c4b20f7c6ecbb247fab82

Named refs at pin: origin/feat/glyph-cache ... feat/color-emoji

Read the diff:

git diff --find-renames b6e820df59a1e9576e5d45a9543fa6689edfb6d6 e6f928144dc6343cb90c4b20f7c6ecbb247fab82

Review concern 03 of 05: Smoke golden with colour emoji (`smoke`)

The why:

[Máté's review](source:s2) asks for an emoji in the golden.

The what:

The smoke Button label gains 👍 and the refreshed golden shows it and the header's 🎮 in colour instead of the missing-glyph box.

Depends on:
- 02 Drawing and measuring by cluster (`text`)

Hunk refs for this concern:
- js/examples/smoke/src/App.tsx
- tests/fixtures/smoke_golden.Linux.png