Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify d105b810b495e10a8d957673abeadf9b8055c125` and `git rev-parse --verify 499076450739a23bece60dc7cdcd09d4828225c8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames d105b810b495e10a8d957673abeadf9b8055c125 499076450739a23bece60dc7cdcd09d4828225c8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  d105b810b495e10a8d957673abeadf9b8055c125

head               499076450739a23bece60dc7cdcd09d4828225c8

Named refs at pin: d105b810b495e10a8d957673abeadf9b8055c125 ... 499076450739a23bece60dc7cdcd09d4828225c8

Read the diff:

git diff --find-renames d105b810b495e10a8d957673abeadf9b8055c125 499076450739a23bece60dc7cdcd09d4828225c8

Review concern 06 of 06: Glossary and SDK README (`docs`)

The why:

The [proposal](source:s1) adds the Runtime Fallback Font term to CONTEXT.md.

The what:

`CONTEXT.md` adds **Runtime Fallback Font** and the raylib fallback of the Runtime Default Font in place, the SDK README names the CJK coverage, and `AGENTS.md` and the build skill say configure downloads the fonts.

Depends on:
- 03 Font registry owns the Runtime Fallback Font (`registry`)

Hunk refs for this concern:
- CONTEXT.md
- js/packages/sdk/README.md
- AGENTS.md
- .agents/skills/vitadeck-build/SKILL.md