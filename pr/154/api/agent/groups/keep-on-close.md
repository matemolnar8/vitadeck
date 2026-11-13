Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2906881a20aa23b2be70b8b0be5d42ceca8b27b7` and `git rev-parse --verify 6fc438bd5a88943893de663f1def580b3ddb0e29` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 2906881a20aa23b2be70b8b0be5d42ceca8b27b7 6fc438bd5a88943893de663f1def580b3ddb0e29 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  2906881a20aa23b2be70b8b0be5d42ceca8b27b7

head               6fc438bd5a88943893de663f1def580b3ddb0e29

Named refs at pin: 2906881a20aa23b2be70b8b0be5d42ceca8b27b7 ... 6fc438bd5a88943893de663f1def580b3ddb0e29

Read the diff:

git diff --find-renames 2906881a20aa23b2be70b8b0be5d42ceca8b27b7 6fc438bd5a88943893de663f1def580b3ddb0e29

Review concern 01 of 01: Keep reviews after close (`keep-on-close`)

The what:

The workflow drops its `pull_request: closed` trigger and `prune --pr` step, and the skill and sites index say 30 days instead of on close.

Attention (tradeoff):
Closing a pull request, merged or not, no longer starts the workflow, so `pr/<n>/` on `gh-pages` stays public until the daily `prune --ttl-days 30` sees a `publishedAt` 30 days old. An abandoned PR's review and evidence screenshots now stay up as long as a merged one's. `prune --pr <n>` by hand is the only faster removal.

Hunk refs for this concern:
- .github/workflows/pages-reviews.yml
- .agents/skills/pages/scripts/pages-review.ts
- .agents/skills/pages/SKILL.md