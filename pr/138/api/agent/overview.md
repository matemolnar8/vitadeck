Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a526711c80571b41bcfc9b420317b70597c23ede` and `git rev-parse --verify a36879835bbd7c67c3b55491523c8dbaf305f373` in this repository.
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

base (merge-base)  a526711c80571b41bcfc9b420317b70597c23ede

head               a36879835bbd7c67c3b55491523c8dbaf305f373

Named refs at pin: origin/main ... origin/t3/ci-c-format

Read the diff:

git diff --find-renames a526711c80571b41bcfc9b420317b70597c23ede a36879835bbd7c67c3b55491523c8dbaf305f373

Commits:
- a368798 fix(scripts): point clang-format.sh at the CI-pinned version
- a526711 Merge pull request #136 from matemolnar8/t3/drop-pr-attribution-clause
- 272bd3f ci: check C formatting of src/ and tests/
- aca107f docs(agents): drop the PR attribution clause

Sources:
- transcript Claude session · Oct 27 Retro on the unattended threads behind PRs #127–#135 lists six vitadeck guardrail fixes.
- pr PR #138 Pinned clang-format job in CI.
  https://github.com/matemolnar8/vitadeck/pull/138
- commit 8cb20a5 Remove clang-format from CI workflow Dropped the CI step because clang-format was unavailable on the macOS runner.

The title:

ci: check C formatting of src/ and tests/

The why:

#119 gated `tests/` with clang-format locally, but CI does not enforce it [retro item 5](source:s1).

The what (trivial):

A `c-format` CI job installs clang-format 21.1.8 from PyPI and runs `scripts/clang-format.sh check`.

## Review concerns

### 01 Pinned clang-format job (`job`)

The Ubuntu `c-format` job installs a pinned clang-format with `pipx` and runs the same script as the `fast` tier, whose not-found hint and `AGENTS.md` name that version.

[groups/job.md](groups/job.md)