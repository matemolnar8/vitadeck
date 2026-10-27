Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify aca107f03473df2cb635ce7f693158515dac9af1` in this repository.
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

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               aca107f03473df2cb635ce7f693158515dac9af1

Named refs at pin: origin/main ... origin/t3/drop-pr-attribution-clause

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 aca107f03473df2cb635ce7f693158515dac9af1

Commits:
- aca107f docs(agents): drop the PR attribution clause

Sources:
- transcript Claude session · Oct 27 Retro on the unattended threads behind PRs #127–#135 lists six vitadeck guardrail fixes.
- pr PR #136 Drops the duplicate attribution clause.
  https://github.com/matemolnar8/vitadeck/pull/136

The title:

docs(agents): drop the PR attribution clause

The why:

PRs ended with three or four attribution lines because Máté's global instructions already add one [retro item 1](source:s1).

The what (trivial):

`AGENTS.md` no longer asks agents to end a PR body with the model and harness that did the work.

## Review concerns

### 01 Pull request body rule (`clause`)

The Body bullet keeps problem, fix and agent-check tier and loses its last sentence.

[groups/clause.md](groups/clause.md)