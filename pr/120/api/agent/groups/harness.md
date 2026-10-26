Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 02d88881c71ebf1091332268ecbcd587dddafb01` and `git rev-parse --verify 538d2833a7caafed2a5042946bfa378f95553eb0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 538d2833a7caafed2a5042946bfa378f95553eb0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  02d88881c71ebf1091332268ecbcd587dddafb01

head               538d2833a7caafed2a5042946bfa378f95553eb0

Named refs at pin: 02d88881c71ebf1091332268ecbcd587dddafb01 ... HEAD

Read the diff:

git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 538d2833a7caafed2a5042946bfa378f95553eb0

Review concern 01 of 04: Harness steps through the demo (`harness`)

Part: Demo goldens

The why:

The goldens should cover [every demo page](source:s1).

The what:

`run_smoke_test.sh` installs the demo with `demo_stubs.js` prepended to `app.js`, and `smoke_harness.c` presses R per `PAGES` entry, then D-pad right twice and Cross to hit "Throw an error", comparing each frame against `tests/fixtures/golden/Linux/`.

Look for:
- Subtle: `PAGES` in `tests/smoke_harness.c` hardcodes the demo's "N/12 · name" headers, so adding, renaming or reordering a demo section times out the harness until `PAGES` and the goldens are updated.
- Subtle: `13-failure-screen.png` paints the stack trace with `app.js` and `runtime.js` line:column positions, so most demo or runtime JS edits also change that golden.
- The old harness asserted `SMOKE_OK` (an interval ticked) and minimum node counts per type; the new harness drops both and relies on pixels, and `demo_stubs.js` freezes the Timers clock.
- `VITADECK_DATA_ROOT` is now the relative `smoke-fixture`, so the Failure Screen path is identical in `out-smoke` and `out-sanitize`.

Hunk refs for this concern:
- tests/smoke_harness.c
- tests/run_smoke_test.sh
- tests/fixtures/demo_stubs.js
- scripts/smoke-docker.sh
- .github/workflows/ci.yml
- tests/fixtures/golden/Linux/01-rect-theme.png
- tests/fixtures/golden/Linux/02-text.png
- tests/fixtures/golden/Linux/03-image.png
- tests/fixtures/golden/Linux/04-button.png
- tests/fixtures/golden/Linux/05-pad-buttons.png
- tests/fixtures/golden/Linux/06-stack.png
- tests/fixtures/golden/Linux/07-scroll.png
- tests/fixtures/golden/Linux/08-text-input.png
- tests/fixtures/golden/Linux/09-storage.png
- tests/fixtures/golden/Linux/10-timers.png
- tests/fixtures/golden/Linux/11-fetch.png
- tests/fixtures/golden/Linux/12-failure-section.png
- tests/fixtures/golden/Linux/13-failure-screen.png
- tests/fixtures/smoke_golden.Linux.png