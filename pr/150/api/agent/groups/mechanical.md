Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995` and `git rev-parse --verify f0d05d344a698b00618fe76f06d9e72e66b01637` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 f0d05d344a698b00618fe76f06d9e72e66b01637 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995

head               f0d05d344a698b00618fe76f06d9e72e66b01637

Named refs at pin: 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 ... t3/retro-nav-docs

Read the diff:

git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 f0d05d344a698b00618fe76f06d9e72e66b01637

Review concern 09 of 09: The implementation moves out of main.c (`mechanical`)

Part: stb_ds once

The why:

`main.c` held the implementation the host binary linked.

The what:

A pure move of the two implementation lines from `src/main.c` into `src/core/stb_ds.c`; safe to skip.

Depends on:
- 06 stb_ds implementation in vitadeck_core (`stb-ds`)

Hunk refs for this concern:
- src/core/stb_ds.c
- src/main.c