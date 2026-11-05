Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995` and `git rev-parse --verify f0d05d344a698b00618fe76f06d9e72e66b01637` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 f0d05d344a698b00618fe76f06d9e72e66b01637 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995

head               f0d05d344a698b00618fe76f06d9e72e66b01637

Named refs at pin: 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 ... t3/retro-nav-docs

Read the diff:

git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 f0d05d344a698b00618fe76f06d9e72e66b01637

Review concern 05 of 09: Republish keeps evidence/ (`pages-evidence`)

Part: Pages evidence

The why:

Refreshing a review replaced `pr/<n>/` whole and deleted the screenshots the PR body links to ([retro](source:s2)).

The what:

`publishPagesReview` clears every entry of the dest folder except `EVIDENCE_DIR` before copying the new export in, and the pages skill and `AGENTS.md` say where screenshots go.

Attention (tradeoff):
Old evidence files are never removed by a republish: a screenshot dropped from the PR body stays hosted until the PR closes and the workflow prunes `pr/<n>/`. A new export's own `evidence/` merges over the kept one, replacing files with the same name.

Hunk refs for this concern:
- .agents/skills/pages/scripts/pages-review.ts
- .agents/skills/pages/SKILL.md
- AGENTS.md@85+85