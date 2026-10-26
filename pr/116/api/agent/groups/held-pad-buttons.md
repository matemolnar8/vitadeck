Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 21f616e174d3a9beedead96ee8bb6df07b9b2bca` and `git rev-parse --verify c603dd347c11466593118be5db42c7483c2608a8` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 21f616e174d3a9beedead96ee8bb6df07b9b2bca c603dd347c11466593118be5db42c7483c2608a8 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  21f616e174d3a9beedead96ee8bb6df07b9b2bca

head               c603dd347c11466593118be5db42c7483c2608a8

Named refs at pin: 21f616e174d3a9beedead96ee8bb6df07b9b2bca ... c603dd347c11466593118be5db42c7483c2608a8

Read the diff:

git diff --find-renames 21f616e174d3a9beedead96ee8bb6df07b9b2bca c603dd347c11466593118be5db42c7483c2608a8

Review concern 03 of 03: Press Pad Buttons held when a Deck App starts (`held-pad-buttons`)

Part: Held Pad Buttons

The why:

The issue asks to treat buttons held at startup as pressed on the first sample or document the edge ([#106](source:s1)); this takes the first option.

The what:

input_clear_focus resets Pad Button states without setting pad_ignore_held, so a held button presses on the next sample unless the Shell hide that preceded the restart already asked to ignore it.

Look for:
- A Runtime Upload Replacement restart while the user holds Circle now sends pressstart to the new runtime; a restart from the Shell keeps ignoring it because hide_shell_to_deck sets the ignore before js_runtime_stop and js_runtime_start run.

Hunk refs for this concern:
- src/ui/input.c@82+82
- src/ui/input.c@295+305
- src/ui/input.h
- tests/input_harness.c@176+177
- CONTEXT.md