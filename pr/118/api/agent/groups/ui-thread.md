Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47` and `git rev-parse --verify 109d347e5c7b68a18469c3a8cfffda8db7e201c1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 109d347e5c7b68a18469c3a8cfffda8db7e201c1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e933a90f2ad45d2abbbbfac04a7b9a987bbad47

head               109d347e5c7b68a18469c3a8cfffda8db7e201c1

Named refs at pin: 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 ... 109d347e5c7b68a18469c3a8cfffda8db7e201c1

Read the diff:

git diff --find-renames 3e933a90f2ad45d2abbbbfac04a7b9a987bbad47 109d347e5c7b68a18469c3a8cfffda8db7e201c1

Review concern 03 of 05: Lay out on the UI thread (`ui-thread`)

The why:

Measuring Text rasterises through the glyph cache, which only the UI thread uses, so the JS thread can no longer run layout in `instance_tree_swap`.

The what:

`laid_out_front()` lays out a new front snapshot once, under `snapshot_mutex`, for the five readers of `layout`: render roots, hit test, focusables, scroll metrics and scroll-at.

Look for:
- Race: `instance_exists`, `instance_is_focusable` and `instance_scroll_for_descendant` read the front snapshot without laying it out; they only read ids, types and parents, never `layout`.

Depends on:
- 02 Text takes its slot in a Stack (`layout`)

Hunk refs for this concern:
- src/ui/instance_tree.c
- src/ui/instance_tree.h