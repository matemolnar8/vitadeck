Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 781ac177fafe6b24c3ec78dd6dc9549eaff15a89` and `git rev-parse --verify 97038f30a2ec905d7c5a64084b981d6bfc03b82c` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 781ac177fafe6b24c3ec78dd6dc9549eaff15a89 97038f30a2ec905d7c5a64084b981d6bfc03b82c -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 02: Catalog bumps within the current major (`deps`)

The why:

The request asks for the latest non-breaking version of each workspace dependency [per the request](source:t1).

The what:

The catalog in `pnpm-workspace.yaml` bumps `@types/node`, `@types/react`, `oxlint`, `rolldown` and `tsx`, and the scaffold template's `@types/react` pin follows the catalog.

Look for:
- `rolldown` goes from `1.0.0-rc.18` to `1.2.15`; it is a runtime dependency of `@vitadeck/sdk`, so authors' `vitadeck build` bundles with it too.
- `oxlint` stops at `1.72.0` because 1.73+ needs `oxlint-tsgolint` 0.24+ or 7.x, outside the pinned `0.23.0` range.

Hunk refs for this concern:
- js/pnpm-workspace.yaml
- js/packages/sdk/cli/templates/scaffold/package.json