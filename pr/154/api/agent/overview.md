Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 2906881a20aa23b2be70b8b0be5d42ceca8b27b7` and `git rev-parse --verify 6fc438bd5a88943893de663f1def580b3ddb0e29` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
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

Commits:
- 6fc438bd ci(pages): a merged pull request's review and evidence stay up for 30 days

Sources:
- pr PR #154
  https://github.com/matemolnar8/vitadeck/pull/154

The title:

A merged pull request's review and evidence stay up for 30 days

The why:

[PR #154](source:s1) says a merged PR's hosted review and screenshots died within a minute of closing, breaking links in its description and the agent's last message.

The what (trivial):

The `Pages reviews` workflow no longer deletes a review when its pull request closes.

## Review concerns

### 01 Keep reviews after close (`keep-on-close`)

The workflow drops its `pull_request: closed` trigger and `prune --pr` step, and the skill and sites index say 30 days instead of on close.

Attention (tradeoff):
Closing a pull request, merged or not, no longer starts the workflow, so `pr/<n>/` on `gh-pages` stays public until the daily `prune --ttl-days 30` sees a `publishedAt` 30 days old. An abandoned PR's review and evidence screenshots now stay up as long as a merged one's. `prune --pr <n>` by hand is the only faster removal.

[groups/keep-on-close.md](groups/keep-on-close.md)