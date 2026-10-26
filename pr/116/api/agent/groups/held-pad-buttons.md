Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3` and `git rev-parse --verify 087217de216a123c2dfed17077b0af7afa71ed6f` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 087217de216a123c2dfed17077b0af7afa71ed6f -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  27683e0ceec2ca0ab7c185c2e97d4f79787f16f3

head               087217de216a123c2dfed17077b0af7afa71ed6f

Named refs at pin: 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 ... 087217de216a123c2dfed17077b0af7afa71ed6f

Read the diff:

git diff --find-renames 27683e0ceec2ca0ab7c185c2e97d4f79787f16f3 087217de216a123c2dfed17077b0af7afa71ed6f

Review concern 02 of 02: Press Pad Buttons held when a Deck App starts (`held-pad-buttons`)

Part: Held Pad Buttons

The why:

The issue asks to treat buttons held at startup as pressed on the first sample or document the edge ([#106](source:s1)); this takes the first option.

The what:

input_clear_focus resets Pad Button states without setting pad_ignore_held, so a held button presses on the next sample unless the Shell hide that preceded the restart already asked to ignore it.

Look for:
- A Runtime Upload Replacement restart while the user holds Circle now sends pressstart to the new runtime; a restart from the Shell keeps ignoring it because hide_shell_to_deck sets the ignore before js_runtime_stop and js_runtime_start run.

Hunk refs for this concern:
- src/ui/input.c@82+82
- src/ui/input.c@295+299
- src/ui/input.h
- tests/input_harness.c@176+177
- CONTEXT.md