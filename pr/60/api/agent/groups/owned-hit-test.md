Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 2d8076f6f21116e13900067cbcf830720ec63a55` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 2d8076f6f21116e13900067cbcf830720ec63a55 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               2d8076f6f21116e13900067cbcf830720ec63a55

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... fix/hit-test-snapshot-lifetime

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 2d8076f6f21116e13900067cbcf830720ec63a55

Review concern 01 of 02: Hit test returns an owned id (`owned-hit-test`)

The why:

The copy must happen while `snapshot_mutex` is held, because `instance_tree_swap` frees the old snapshot right after unlocking.

The what:

`instance_hit_test` in `instance_tree.c` now `strdup`s the hit id inside the lock, matching `instance_scroll_at`, and the header changes the return type to `char *`.

Hunk refs for this concern:
- src/ui/instance_tree.c
- src/ui/instance_tree.h