Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 2d8076f6f21116e13900067cbcf830720ec63a55` in this repository.
   Done when both objects exist.

2. Choose the relevant review concerns.
   Read Review concerns. Fetch a concern file only when that concern is relevant to the question.
   Done when every concern the question touches has its markdown loaded.

3. Answer from live git.
   Follow those files. Use the why and the what as interpretation. Live git wins when they disagree.
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

Commits:
- 2d8076f fix(ui): copy hit-test result before releasing snapshot lock

Sources:
- ticket #39 Return an owned copy of the hit id and free it in input code; add an ASan harness.
  https://github.com/matemolnar8/vitadeck/issues/39

The title:

fix(ui): copy hit-test result before releasing snapshot lock

The why:

[`instance_hit_test` returned a pointer into a snapshot the JS thread can free](source:s1) while the input code still used it.

The what (trivial):

`instance_hit_test` returns an owned `strdup` copy of the hit id, and the mouse and touch pollers free it at the end of each frame.

Look for:
- [#39](source:s1) asks for a CTest harness that swaps snapshots while hit-testing under ASan; the PR adds none, because without ASan (#53) such a test cannot catch the use-after-free deterministically.

## Review concerns

### 01 Hit test returns an owned id (`owned-hit-test`)

`instance_hit_test` in `instance_tree.c` now `strdup`s the hit id inside the lock, matching `instance_scroll_at`, and the header changes the return type to `char *`.

[groups/owned-hit-test.md](groups/owned-hit-test.md)

### 02 Input pollers free the hit id (`input-frees`)

`poll_mouse_input` and `poll_touch_input` hold `top_id` as `char *` and free it on their single exit path at the end of the function.

Depends on:
- 01 Hit test returns an owned id (`owned-hit-test`)

[groups/input-frees.md](groups/input-frees.md)