Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 8944d5cdb7fc7fc450617cca3781bb2cf1808471` and `git rev-parse --verify b6e820df59a1e9576e5d45a9543fa6689edfb6d6` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 b6e820df59a1e9576e5d45a9543fa6689edfb6d6 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  8944d5cdb7fc7fc450617cca3781bb2cf1808471

head               b6e820df59a1e9576e5d45a9543fa6689edfb6d6

Named refs at pin: origin/feat/stack-layout ... origin/feat/glyph-cache

Read the diff:

git diff --find-renames 8944d5cdb7fc7fc450617cca3781bb2cf1808471 b6e820df59a1e9576e5d45a9543fa6689edfb6d6

Review concern 04 of 05: Button font and fontSize (`button`)

The why:

The [design](source:s1) gives Button the font props Text already has.

The what:

`VitaButtonProps` gains `font`/`fontSize`, the reconciler passes them after the label, and `read_button_props` stores the font name that `render_button_instance` draws with.

Look for:
- Breaking (internal): the native Button now takes 22 arguments, with the font name at index 10. A `runtime.js` from before this change makes every Button create and update return early. The runtime and host ship together.
- An unset `fontSize` now passes 0 and the host applies `VD_BUTTON_FONT_SIZE_DEFAULT` (20), where the reconciler used to hard-code 20.

Depends on:
- 03 Text drawing through the cache (`render`)

Hunk refs for this concern:
- js/packages/sdk/src/vitadeck-host-types.ts
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts
- src/jslib/instance.c
- src/ui/instance_tree.h
- src/ui/instance_tree.c
- src/ui/render.c@338+345