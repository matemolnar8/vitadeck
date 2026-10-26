Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 781ac177fafe6b24c3ec78dd6dc9549eaff15a89` and `git rev-parse --verify 97038f30a2ec905d7c5a64084b981d6bfc03b82c` in this repository.
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

base (merge-base)  781ac177fafe6b24c3ec78dd6dc9549eaff15a89

head               97038f30a2ec905d7c5a64084b981d6bfc03b82c

Named refs at pin: 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 ... chore/upgrade-pnpm-js-deps

Read the diff:

git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 97038f30a2ec905d7c5a64084b981d6bfc03b82c

Commits:
- 97038f3 chore(js): upgrade pnpm to 12.15.0 and dependencies within major

Sources:
- transcript Claude Code session · Oct 26 Máté asks to move packageManager to the latest pnpm, adapt config to pnpm 11/12 defaults, keep Docker/CI pins in sync, bump deps within their major, and list skipped majors in the PR.

The title:

chore(js): upgrade pnpm and dependencies

The why:

Machines with pnpm 12 fail `pnpm --dir js build` with `ERR_PNPM_BAD_PM_VERSION` because `packageManager` pins pnpm 10.28.2 [per the request](source:t1).

The what (small):

The JS workspace moves from pnpm 10.28.2 to pnpm 12.15.0, and five catalog pins move to their latest version inside the current semver range.

## Review concerns

### 01 pnpm 12 and its new defaults (`pnpm`)

`packageManager` and the Dockerfile's `corepack prepare` both pin pnpm 12.15.0, and `allowBuilds: { esbuild: false }` in `pnpm-workspace.yaml` keeps the install from failing on esbuild's postinstall.

[groups/pnpm.md](groups/pnpm.md)

### 02 Catalog bumps within the current major (`deps`)

The catalog in `pnpm-workspace.yaml` bumps `@types/node`, `@types/react`, `oxlint`, `rolldown` and `tsx`, and the scaffold template's `@types/react` pin follows the catalog.

[groups/deps.md](groups/deps.md)