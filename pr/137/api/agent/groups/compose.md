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

Review concern 03 of 03: docker compose, not docker-compose (`compose`)

The why:

`docker-compose` is not installed on this Mac while `docker compose` is [retro item 4](source:s1).

The what:

The `vitadeck-build` skill and `AGENTS.md` switch every `docker-compose run` to `docker compose run`, and `run_vita` drops its fallback.

Hunk refs for this concern:
- .agents/skills/vitadeck-build/SKILL.md
- scripts/agent-check.sh@82+108
- AGENTS.md@64+64