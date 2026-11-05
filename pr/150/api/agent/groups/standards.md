Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995` and `git rev-parse --verify b0a30605c0c10bd3393bb263b9ce286e06f29813` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 b0a30605c0c10bd3393bb263b9ce286e06f29813 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995

head               b0a30605c0c10bd3393bb263b9ce286e06f29813

Named refs at pin: 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 ... t3/retro-nav-docs

Read the diff:

git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 b0a30605c0c10bd3393bb263b9ce286e06f29813

Review concern 08 of 11: CODING_STANDARDS.md (`standards`)

Part: Review standards

The why:

Review delegates were handed `AGENTS.md` as the standards file because the repo had none ([retro](source:s2)).

The what:

The "Hit every surface" rules move from `AGENTS.md` into `CODING_STANDARDS.md` with new ADR, manifest and glossary checks, and `AGENTS.md` keeps a one-paragraph pointer.

Attention (tradeoff):
Implementing agents no longer see the surface rules in their always-loaded context, only the pointer naming them. Reviews now carry those rules, so a miss surfaces in review rather than during implementation.

Hunk refs for this concern:
- CODING_STANDARDS.md
- AGENTS.md@51+51