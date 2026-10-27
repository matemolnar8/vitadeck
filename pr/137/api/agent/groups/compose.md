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

Review concern 03 of 03: docker compose, not docker-compose (`compose`)

The why:

`docker-compose` is not installed on this Mac while `docker compose` is [retro item 4](source:s1).

The what:

The `vitadeck-build` skill and `AGENTS.md` switch every `docker-compose run` to `docker compose run`, and `run_vita` drops its fallback.

Hunk refs for this concern:
- .agents/skills/vitadeck-build/SKILL.md
- scripts/agent-check.sh@82+116
- AGENTS.md@64+64