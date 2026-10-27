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

Review concern 01 of 03: Run on the pinned Node (`node`)

The why:

The shell's default Node on this Mac is 22, and nothing stopped a check from running on it [retro item 4](source:s1).

The what:

`use_pinned_node` compares the running Node major with `.nvmrc` before `fast`, `host` and `full`, and `engineStrict` makes `pnpm install` enforce `engines.node`.

Look for:
- Subtle: with fnm on `PATH`, a Node 22 shell does not fail; the script `exec`s `fnm exec --using=.nvmrc` on itself once, guarded by `VITADECK_PINNED_NODE`, so the whole tier silently runs on Node 24 after one stderr line.
- `smoke` is not in the `case`, so it never re-execs; it runs Node inside Docker.

Hunk refs for this concern:
- scripts/agent-check.sh@25+25
- scripts/agent-check.sh@82+108
- js/pnpm-workspace.yaml
- AGENTS.md@33+33