Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify fcb9c6928d3a15b16fffaea3d7185875f5370fc2` and `git rev-parse --verify 6adf657dd6eff69bf072e747e9189193259b547c` in this repository.
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

base (merge-base)  fcb9c6928d3a15b16fffaea3d7185875f5370fc2

head               6adf657dd6eff69bf072e747e9189193259b547c

Named refs at pin: fcb9c692 ... HEAD

Read the diff:

git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 6adf657dd6eff69bf072e747e9189193259b547c

Commits:
- 6adf657d test(bench): re-record the Vita baseline with minesweeper-shell
- b620259d fix(bench): initialize the shell field on every scenario
- f869d239 test(bench): re-record the Vita3K baseline with the final blur
- 1ad4ccae test(smoke): refresh the Shell golden for downsample-only blur
- d6a6fbf6 perf(shell): blur with downsampling alone, clamped at the edges
- 976e4f1a test(bench): re-record the Vita3K baseline with minesweeper-shell
- cb8c3012 test(smoke): refresh the Shell golden for the blurred backdrop
- 3cfe2e0e feat(shell): blur the Deck App behind the VitaDeck Shell
- 5c1dd9b2 test(bench): add a minesweeper-shell scenario with the Shell open

Sources:
- pr PR #152 Blur the Deck App behind the Shell with a downsample chain; Vita3K and host benchmarks show about +0.2 ms CPU per Shell frame.
  https://github.com/matemolnar8/vitadeck/pull/152
- transcript Claude Code session · Nov 5 Add blur behind the VitaDeck Shell (currently just transparent); benchmark to make sure it is not incurring an unnecessarily large performance cost.

The title:

feat(shell): blur the Deck App behind the VitaDeck Shell

The why:

The Shell was [just transparent](source:s2) over the Deck App, and Máté asked for a blur behind it that does not [incur an unnecessarily large performance cost](source:s2).

The what (small):

While the Shell is open, the Deck App canvas renders into a render texture, `src/ui/blur.c` box-downsamples it to 1/16, and the result is stretched back under the Shell. A new `minesweeper-shell` Benchmark Mode scenario measures that frame.

Things to check:
- The request asks to [make sure the cost isn't unnecessarily large](source:s2). On the Vita the blur adds about 4 ms of UI work per Shell frame, with no dropped frames. A frozen-backdrop variant measured cheaper than no blur at all ([PR #152](source:s1)), but it stops Deck App animation behind the Shell, so it is not in this PR.

## Review concerns

### 01 Blur the Deck App behind the Shell (`blur`)

`draw_frame` in `src/main.c` and the smoke harness wrap `bootstrap_draw_deck_canvas` in `blur_begin`/`blur_end` while the Shell is visible, and `src/ui/blur.c` halves the captured canvas four times before stretching the 60×34 level over the screen.

Attention (tradeoff):
With the Shell open, the Vita spends 3.85 ms more wall time per frame in the `shell` span, and UI thread CPU rises from 38% to 48.6%. FPS stays at 59.4 with no dropped frames, but only about 7 ms of slack remains. `draw_frame` re-renders and re-blurs the Deck App every Shell frame so live apps stay live behind the tint; a backdrop frozen when the Shell opens measured 2.5 ms of UI work and 13% CPU.

[groups/blur.md](groups/blur.md)

### 02 Benchmark the Shell frame (`bench`)

The `shell` flag on `BenchScenario` makes `bench_shell_open` true, and `main.c` presses Start for the Shell whenever that differs from `shell_is_visible`, so `minesweeper-shell` measures the Shell over Minesweeper and later scenarios get it closed.

Attention (risk):
The re-recorded Vita3K baseline moves every scenario, not only `minesweeper-shell`. The PR says the base build shows the same drift on that Mac, so the old numbers no longer described it. The device baseline moves within a few percent and gains `minesweeper-shell`.

[groups/bench.md](groups/bench.md)