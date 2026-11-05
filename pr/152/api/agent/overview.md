Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify fcb9c6928d3a15b16fffaea3d7185875f5370fc2` and `git rev-parse --verify f869d239cde93bf2f60ff7fe25f3c0e22a770d28` in this repository.
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

head               f869d239cde93bf2f60ff7fe25f3c0e22a770d28

Named refs at pin: fcb9c692 ... HEAD

Read the diff:

git diff --find-renames fcb9c6928d3a15b16fffaea3d7185875f5370fc2 f869d239cde93bf2f60ff7fe25f3c0e22a770d28

Commits:
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

Look for:
- The request asks to [benchmark the cost](source:s2). The PR measures it in Vita3K and on the host under llvmpipe ([PR #152](source:s1)), and neither models the Vita's GPU. The device run (`scripts/bench.sh vita`) has not happened, so the Vita fill cost of the extra render targets is unmeasured.

## Review concerns

### 01 Blur the Deck App behind the Shell (`blur`)

`draw_frame` in `src/main.c` and the smoke harness wrap `bootstrap_draw_deck_canvas` in `blur_begin`/`blur_end` while the Shell is visible, and `src/ui/blur.c` halves the captured canvas four times before stretching the 60×34 level over the screen.

Attention (tradeoff):
Each Shell frame now costs six render targets (full-size canvas, four 2x downsamples, the screen), each a GXM scene on the Vita. Dropping the upsample chain cut three scenes; the stretched 60×34 level is slightly blockier but sits under the Shell's alpha-200 tint. With the Shell hidden, `draw_frame` takes the old path.

[groups/blur.md](groups/blur.md)

### 02 Benchmark the Shell frame (`bench`)

The `shell` flag on `BenchScenario` makes `bench_shell_open` true, and `main.c` presses Start for the Shell whenever that differs from `shell_is_visible`, so `minesweeper-shell` measures the Shell over Minesweeper and later scenarios get it closed.

Attention (risk):
The re-recorded Vita3K baseline moves every scenario, not only `minesweeper-shell`. The PR says the base build shows the same drift on that Mac, so the old numbers no longer described it. The device baseline has no `minesweeper-shell` entry yet.

[groups/bench.md](groups/bench.md)