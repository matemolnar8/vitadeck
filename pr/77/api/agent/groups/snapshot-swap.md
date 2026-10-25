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

Review concern 03 of 07: Snapshot swap stress harness (`snapshot-swap`)

Part: Native harnesses

The why:

[#39](source:s2) asked for a harness that swaps snapshots while hit-testing under ASan.

The what:

The main thread mutates and swaps 20000 snapshots while a reader thread calls every front-snapshot accessor and checks the ids it copies out.

Look for:
- Moving the `copy_id` in `instance_hit_test` after the unlock makes this harness fail under ASan with heap-use-after-free, so it does catch a #39 regression.

Depends on:
- 02 Back-tree fixture (`fixture`)

Hunk refs for this concern:
- tests/snapshot_swap_harness.c
- CMakeLists.txt@242+243