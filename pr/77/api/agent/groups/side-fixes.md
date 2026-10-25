Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 4d6d7c5a7816464240a173bb7185dcb1c8090429` and `git rev-parse --verify 56d8e56692382c5f4de8a8472ee1cb72238466f4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 4d6d7c5a7816464240a173bb7185dcb1c8090429 56d8e56692382c5f4de8a8472ee1cb72238466f4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  4d6d7c5a7816464240a173bb7185dcb1c8090429

head               56d8e56692382c5f4de8a8472ee1cb72238466f4

Named refs at pin: origin/build/warnings-sanitizers-js-limits ... test/native-harnesses

Read the diff:

git diff --find-renames 4d6d7c5a7816464240a173bb7185dcb1c8090429 56d8e56692382c5f4de8a8472ee1cb72238466f4

Review concern 07 of 07: AGENTS.md path and docker compose fallback (`side-fixes`)

Part: Side fixes

The why:

The brief asked for both while in these files: `AGENTS.md` named a `jimp.h` path that does not exist, and the Mac has only `docker compose`.

The what:

`AGENTS.md` points at `vendor/jimp/jimp.h`, and `run_vita` in `agent-check.sh` tries `docker compose` before `docker-compose`.

Hunk refs for this concern:
- AGENTS.md
- scripts/agent-check.sh