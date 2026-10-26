Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify c7a4133c52f8c8fa61a0fcba4283dd7dc456c116` and `git rev-parse --verify 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0` in this repository.
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

base (merge-base)  c7a4133c52f8c8fa61a0fcba4283dd7dc456c116

head               2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Named refs at pin: c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 ... HEAD

Read the diff:

git diff --find-renames c7a4133c52f8c8fa61a0fcba4283dd7dc456c116 2e0ae62408e1b95b9b9d4d0a7620cfcf728694a0

Commits:
- 2e0ae62 test(bench): record the Vita3K baseline after the drawing changes
- 220d94d Merge remote-tracking branch 'origin/main' into t3/perf-native-drawing-126
- 3e5d699 docs(render): text_lines is broken once by layout, which keeps the copy render draws
- c691069 Merge remote-tracking branch 'origin/main' into t3/perf-native-drawing-126
- c8cadbe perf(render): draw a Text's lines from layout instead of breaking them every frame
- 2ed81e2 perf(render): fill rounded rects from precomputed corner directions
- fc02f91 perf(render): draw shapes from the glyph atlas so they batch with text

Sources:
- ticket #126 Draw takes 6-10 ms of the 16.7 ms frame on the device; profile by element type and make draw mean and UI thread CPU drop in every scenario.
  https://github.com/matemolnar8/vitadeck/issues/126
- pr PR #130 A Vita3K profile located the cost in rounded Button fills, per-frame text_lines and texture-switch draw calls; draw mean drops 51-74% in Vita3K.
  https://github.com/matemolnar8/vitadeck/pull/130

The title:

perf(render): cut native draw time on rounded rects, text lines and batching

The why:

On the Vita the UI thread spends most of each frame drawing, even when nothing changes, so richer Deck Apps will start dropping frames [#126](source:s1).

The what (medium):

Three independent cuts to the UI thread's draw span: rounded rect fills skip trigonometry, a Text's lines are broken once by layout instead of every frame, and shapes sample the glyph atlas so rlgl batches them with text. The Vita3K baseline is re-recorded.

Look for:
- The issue's Verify step asks for `scripts/bench.sh vita` with UI thread CPU % on the device [#126](source:s1); the PR measures only Vita3K, which has no CPU %, and leaves the Vita baseline untouched [PR #130](source:s2).
- The issue proposes skipping or reusing unchanged frames [#126](source:s1); the PR does not, and cuts per-frame cost instead.

## Review concerns

### 01 Rounded rect fills without trig (`rounded`)

`draw_rounded_rect` in `render.c` replays raylib 5.5's quad path from a corner-direction table built once per segment count, and the four rounded fill call sites switch to it.

[groups/rounded.md](groups/rounded.md)

### 02 Text lines kept from layout (`text-lines`)

`layout_tree` takes the front snapshot's arena and keeps each Text's lines as a `TextLayout` in `InstanceLayout.text`, which `render_text_instance` draws instead of calling `text_lines`.

[groups/text-lines.md](groups/text-lines.md)

### 03 Shapes batch with text through the glyph atlas (`atlas-shapes`)

Every atlas page in `glyph_cache.c` starts with an opaque white square, and `text_upload_pages` points raylib's shapes texture at page 0's, restoring raylib's in `text_reset` before the pages unload.

[groups/atlas-shapes.md](groups/atlas-shapes.md)

### 04 Vita3K baseline (`baseline`)

Regenerated Benchmark Mode output from this branch on the same machine as main's baseline; the numbers are the PR's proof, so skim rather than read.

[groups/baseline.md](groups/baseline.md)