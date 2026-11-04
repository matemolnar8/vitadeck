Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 05cd3b580fb6c9be2272de4548816decc5f3de40` and `git rev-parse --verify 92759426382c6cafaf057cbe8556ffe04ec30dd4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 92759426382c6cafaf057cbe8556ffe04ec30dd4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  05cd3b580fb6c9be2272de4548816decc5f3de40

head               92759426382c6cafaf057cbe8556ffe04ec30dd4

Named refs at pin: 05cd3b58 ... 92759426

Read the diff:

git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 92759426382c6cafaf057cbe8556ffe04ec30dd4

Review concern 05 of 06: Demo Sound section and the tests that count sections (`demo`)

The why:

The [request](source:s2) asks for a demo section; inserting it fourth shifts every index the smoke test, input harness and benchmark rely on.

The what:

`SoundSection` plays the chime and toggles the MP3 loop, and inserting it after Image renumbers the smoke pages and goldens, adds an input-harness round and one `r` to `demo-scroll`.

Attention (tradeoff):
`demo-scroll` still measures the Scroll section, now with a 13-button nav list. The Vita3K and Vita baselines in `tests/bench/baselines/` were not re-recorded, so the next comparison against them includes that small extra render cost.

Depends on:
- 04 `playSound` and `stopSound` in the SDK (`sdk`)

Hunk refs for this concern:
- js/examples/demo/src/sections/SoundSection.tsx
- js/examples/demo/src/App.tsx
- js/examples/demo/vitadeck.config.json
- js/examples/demo/README.md
- tests/smoke_harness.c
- tests/deck_app_input_harness.c
- src/core/bench.c
- tests/fixtures/golden/Linux/01-rect-theme.png
- tests/fixtures/golden/Linux/02-text.png
- tests/fixtures/golden/Linux/03-image.png
- tests/fixtures/golden/Linux/04-button.png
- tests/fixtures/golden/Linux/04-sound.png
- tests/fixtures/golden/Linux/05-button.png
- tests/fixtures/golden/Linux/05-pad-buttons.png
- tests/fixtures/golden/Linux/06-pad-buttons.png
- tests/fixtures/golden/Linux/06-stack.png
- tests/fixtures/golden/Linux/07-scroll.png
- tests/fixtures/golden/Linux/07-stack.png
- tests/fixtures/golden/Linux/08-scroll.png
- tests/fixtures/golden/Linux/08-text-input.png
- tests/fixtures/golden/Linux/09-storage.png
- tests/fixtures/golden/Linux/09-text-input.png
- tests/fixtures/golden/Linux/10-storage.png
- tests/fixtures/golden/Linux/10-timers.png
- tests/fixtures/golden/Linux/11-fetch.png
- tests/fixtures/golden/Linux/11-timers.png
- tests/fixtures/golden/Linux/12-failure-section.png
- tests/fixtures/golden/Linux/12-fetch.png
- tests/fixtures/golden/Linux/13-failure-section.png