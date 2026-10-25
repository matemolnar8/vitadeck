Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 3e1dd092b1939420d40648029dee3881f9e4842b` and `git rev-parse --verify b31c0d62da5f6d3d7dd8ae66c1f968e692df5619` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  3e1dd092b1939420d40648029dee3881f9e4842b

head               b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Named refs at pin: 3e1dd092b1939420d40648029dee3881f9e4842b ... b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Read the diff:

git diff --find-renames 3e1dd092b1939420d40648029dee3881f9e4842b b31c0d62da5f6d3d7dd8ae66c1f968e692df5619

Review concern 01 of 07: autoFocus channel in the instance tree (`focus-request`)

The why:

Focus state lives on the UI thread, while `autoFocus` arrives on the JS thread; this group gives the groups below a thread-safe hand-off.

The what:

`instance_back_request_focus` stores an id in the back tree, `instance_tree_swap` publishes it with the snapshot under `snapshot_mutex`, and `instance_take_focus_request` hands it to the UI thread once; `instance_is_button` lets hover ignore Rects and Scrolls.

Look for:
- Subtle: a request published for an instance that is gone by the time the UI thread takes it is dropped silently, and `instance_tree_clear` forgets both back and front requests.

Hunk refs for this concern:
- src/ui/instance_tree.c
- src/ui/instance_tree.h@112+114