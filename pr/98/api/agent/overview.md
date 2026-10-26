Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify 978568cb5b3b89bc65e656c665ead432947b05c6` in this repository.
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

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               978568cb5b3b89bc65e656c665ead432947b05c6

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 978568cb5b3b89bc65e656c665ead432947b05c6

Commits:
- 978568c Merge origin/main into feat/scroll-in-stack
- 76eef5b feat(layout): Scroll flows inside Stack; transparent Stacks pass hits through

Sources:
- ticket #89 Let Scroll participate in Stack flow with an explicit height or fill the rest as last child; decide Stack hit transparency; extend layout_harness.
  https://github.com/matemolnar8/vitadeck/issues/89
- pr PR #98 Scroll flows in Stack, unfilled Stack is hit-transparent, Scroll props optional, timers example uses the pattern.
  https://github.com/matemolnar8/vitadeck/pull/98

The title:

Scroll flows inside Stack; transparent Stacks pass hits through

The why:

A fixed header above a scrolling list in one column still needed hand-placed coordinates, because a `Scroll` child kept its own `x`/`y` ([#89](source:s1)).

The what (small):

`layout_flow` now places `Scroll` children like any other flowed child, and a last-child Scroll with no main-axis size fills the rest of a sized parent. An unfilled `Stack` stops taking hits itself, and the SDK makes Scroll's position and size optional.

Look for:
- Subtle: a `Scroll` nested directly in a `Scroll` now flows too, because a Scroll is a column Stack; [#89](source:s1) only names Stack parents, and the nested-scroll fixture in `tests/hit_test_harness.c` was reordered to keep its expectations.
- Subtle: tapping the gap inside an unfilled `Stack` that sits in a `Scroll` now returns the Scroll's id instead of the Stack's, so a drag that starts there still scrolls.

## Review concerns

### 01 Scroll in the Stack flow (`layout`)

`flows()` now accepts `NT_SCROLL`, and `fills_rest()` lets `layout_flow` overwrite a last-child Scroll's fitted main size with what the parent's stated size leaves, which the new `layout_harness` cases pin down.

[groups/layout.md](groups/layout.md)

### 02 Hit-transparent Stacks (`hit`)

`hit_test_instance` returns a Stack's id only when it has a fill, and `hit_test_harness` checks both kinds plus the nested-scroll fixture that now flows.

Depends on:
- 01 Scroll in the Stack flow (`layout`)

[groups/hit.md](groups/hit.md)

### 03 SDK contract and docs (`sdk`)

`VitaScrollProps` gains optional `x`/`y`/`width`/`height`, the reconciler defaults them to 0 for the unchanged native call, and `CONTEXT.md`, the README and the JSDoc describe the flow and hit rules.

Depends on:
- 01 Scroll in the Stack flow (`layout`)
- 02 Hit-transparent Stacks (`hit`)

[groups/sdk.md](groups/sdk.md)

### 04 Timers example (`example`)

The timers app becomes a sized column whose three demo rows sit above a `Scroll` that fills the rest with a tick log derived from `ticks`.

Depends on:
- 03 SDK contract and docs (`sdk`)

[groups/example.md](groups/example.md)