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

Review concern 01 of 02: pnpm 12 and its new defaults (`pnpm`)

The why:

pnpm 12 is the version on the dev machines [per the request](source:t1), and pnpm 11 turned `strictDepBuilds` on by default.

The what:

`packageManager` and the Dockerfile's `corepack prepare` both pin pnpm 12.15.0, and `allowBuilds: { esbuild: false }` in `pnpm-workspace.yaml` keeps the install from failing on esbuild's postinstall.

Look for:
- Subtle: `esbuild: false` keeps pnpm 10's behaviour, where esbuild's postinstall was already skipped; without the entry pnpm 12 stops `pnpm install` with `ERR_PNPM_IGNORED_BUILDS`.
- Subtle: the Dockerfile still uses `corepack prepare`, which only works because nodesource's Node 22 now ships corepack 0.36; corepack before 0.35 looks for `bin/pnpm.cjs` and crashes, which the new `AGENTS.md` setup line documents.

Hunk refs for this concern:
- js/package.json
- Dockerfile
- js/pnpm-workspace.yaml
- AGENTS.md