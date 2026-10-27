Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2` and `git rev-parse --verify d6edb09cfe8f79d79fd4782ff75a48db7f7ee98c` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 d6edb09cfe8f79d79fd4782ff75a48db7f7ee98c -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2

head               d6edb09cfe8f79d79fd4782ff75a48db7f7ee98c

Named refs at pin: origin/main ... origin/t3/agent-check-node-sdk

Read the diff:

git diff --find-renames 9ebcde7346d590fe7a7f43160760ca3e7d6d2bc2 d6edb09cfe8f79d79fd4782ff75a48db7f7ee98c

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
- scripts/agent-check.sh@39+56
- scripts/agent-check.sh@56+80
- AGENTS.md@64+64