Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a526711c80571b41bcfc9b420317b70597c23ede` and `git rev-parse --verify 2c2415d15f7bebbe8aa7af46f9eb3b6be1f3072c` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a526711c80571b41bcfc9b420317b70597c23ede 2c2415d15f7bebbe8aa7af46f9eb3b6be1f3072c -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a526711c80571b41bcfc9b420317b70597c23ede

head               2c2415d15f7bebbe8aa7af46f9eb3b6be1f3072c

Named refs at pin: origin/main ... origin/t3/agent-check-node-sdk

Read the diff:

git diff --find-renames a526711c80571b41bcfc9b420317b70597c23ede 2c2415d15f7bebbe8aa7af46f9eb3b6be1f3072c

Review concern 02 of 03: Install and build JS before tsc (`build`)

The why:

`tsc` reads the SDK `dist` and the `.vitadeck/` font types that only `vitadeck build` writes, so a fresh tree failed with `"mono"` errors [retro item 4](source:s1).

The what:

`run_fast` and `run_host` call `install_js`, and `run_fast` runs `pnpm --dir js build` before the typecheck.

Look for:
- Perf: `fast` now costs about 6 s more for the JS build; a no-op `pnpm install --frozen-lockfile` takes 36 ms.
- Subtle: `--frozen-lockfile` makes `fast` fail when `package.json` changed without a lockfile update, where it used to pass.

Depends on:
- 01 Run on the pinned Node (`node`)

Hunk refs for this concern:
- scripts/agent-check.sh@8+8
- scripts/agent-check.sh@39+64
- scripts/agent-check.sh@56+88
- AGENTS.md@64+64