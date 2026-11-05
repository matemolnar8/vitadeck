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

Review concern 04 of 11: Shared demo section count (`demo-sections`)

Part: Demo section count

The why:

Adding the Sound section left `ROUNDS` and the bench `demo-scroll` keys stale with every test passing; only a reviewer caught it ([retro](source:s2)).

The what:

`ROUNDS` in `deck_app_input_harness.c` comes from `DEMO_SECTION_COUNT`, `smoke_harness.c` static-asserts its `PAGES` length against it, and a comment above `SECTIONS` in the demo names every file that changes with a section.

Hunk refs for this concern:
- tests/demo_sections.h
- tests/deck_app_input_harness.c
- tests/smoke_harness.c@39+38
- js/examples/demo/src/App.tsx
- .agents/skills/vitadeck-smoke/SKILL.md
- AGENTS.md@28+28