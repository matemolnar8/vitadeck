Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ece6f2c3bf4d9da1a964534b195c30231cc89a18` and `git rev-parse --verify 173943acf944d4c4ad2aa50f99a6a2933fcbac67` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ece6f2c3bf4d9da1a964534b195c30231cc89a18 173943acf944d4c4ad2aa50f99a6a2933fcbac67 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ece6f2c3bf4d9da1a964534b195c30231cc89a18

head               173943acf944d4c4ad2aa50f99a6a2933fcbac67

Named refs at pin: origin/main ... origin/t3/deck-app-input-harness

Read the diff:

git diff --find-renames ece6f2c3bf4d9da1a964534b195c30231cc89a18 173943acf944d4c4ad2aa50f99a6a2933fcbac67

Review concern 04 of 04: Docs follow the label (`docs`)

The why:

AGENTS.md, ADR 0002 and the smoke skill named `smoke_harness` as the excluded test, which became inaccurate.

The what:

The docs swap `smoke_harness` for the `smoke` label and describe the input harness; wording only.

Depends on:
- 03 Select smoke tests by label (`label-wiring`)

Hunk refs for this concern:
- .agents/skills/vitadeck-smoke/SKILL.md
- AGENTS.md
- docs/adr/0002-smoke-ci-linux-xvfb.md