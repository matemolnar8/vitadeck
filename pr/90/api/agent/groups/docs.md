Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8944d5cdb7fc7fc450617cca3781bb2cf1808471` and `git rev-parse --verify b6e820df59a1e9576e5d45a9543fa6689edfb6d6` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 b6e820df59a1e9576e5d45a9543fa6689edfb6d6 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  8944d5cdb7fc7fc450617cca3781bb2cf1808471

head               b6e820df59a1e9576e5d45a9543fa6689edfb6d6

Named refs at pin: origin/feat/stack-layout ... origin/feat/glyph-cache

Read the diff:

git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 b6e820df59a1e9576e5d45a9543fa6689edfb6d6

Review concern 05 of 05: Glossary and SDK README (`docs`)

The why:

AGENTS.md asks for in-place doc fixes when behaviour changes; per-character fallback is new author-visible behaviour.

The what:

The **Deck App Font** and **Runtime Default Font** entries, and one SDK README paragraph, describe the fallback and the Button font props.

Depends on:
- 03 Text drawing through the cache (`render`)

Hunk refs for this concern:
- CONTEXT.md
- js/packages/sdk/README.md