Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3f9ba8c6b32fb89565efa153fadd4a1a876396ee` and `git rev-parse --verify e7d27d003d9cf91aaf9645382cb60facbc614bf0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee e7d27d003d9cf91aaf9645382cb60facbc614bf0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3f9ba8c6b32fb89565efa153fadd4a1a876396ee

head               e7d27d003d9cf91aaf9645382cb60facbc614bf0

Named refs at pin: 3f9ba8c6b32fb89565efa153fadd4a1a876396ee ... HEAD

Read the diff:

git diff --find-renames 3f9ba8c6b32fb89565efa153fadd4a1a876396ee e7d27d003d9cf91aaf9645382cb60facbc614bf0

Review concern 03 of 04: clang-format covers tests/ (`gate`)

Part: Format gate

The why:

[`scripts/clang-format.sh` only checks `src/`](source:s1).

The what:

Both `check` and `format` in `scripts/clang-format.sh` now find files under `src` and `tests`, and `AGENTS.md` and the `vitadeck-build` skill say the same.

Hunk refs for this concern:
- scripts/clang-format.sh
- AGENTS.md
- .agents/skills/vitadeck-build/SKILL.md