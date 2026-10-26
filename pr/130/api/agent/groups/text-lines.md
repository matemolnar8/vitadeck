Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify c7a4133c52f8c8fa61a0fcba4283dd7dc456c116` and `git rev-parse --verify 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  c7a4133c52f8c8fa61a0fcba4283dd7dc456c116

head               2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Named refs at pin: c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 ... HEAD

Read the diff:

git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Review concern 02 of 04: Text lines kept from layout (`text-lines`)

The why:

Render re-joined, re-wrapped and re-measured every Text every frame, 1.1-1.6 ms of demo-text's and demo-scroll's draw span [PR #130](source:s2).

The what:

`layout_tree` takes the front snapshot's arena and keeps each Text's lines as a `TextLayout` in `InstanceLayout.text`, which `render_text_instance` draws instead of calling `text_lines`.

Look for:
- Subtle: the kept lines live in the snapshot arena, which the JS thread frees after a swap; render reads them only under `snapshot_mutex`, and fonts reload only after `instance_tree_clear`, so a drawn Text never outlives its lines or font.

Hunk refs for this concern:
- src/ui/instance_tree.c
- src/ui/instance_tree.h
- src/ui/layout.c
- src/ui/layout.h
- src/ui/render.c@109+195
- src/ui/text.h
- CMakeLists.txt
- tests/layout_harness.c