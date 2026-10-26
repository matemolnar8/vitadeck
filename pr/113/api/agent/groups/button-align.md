Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify c45708bc4894acd44d9184265a1624cda104eeb5` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 c45708bc4894acd44d9184265a1624cda104eeb5 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               c45708bc4894acd44d9184265a1624cda104eeb5

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 c45708bc4894acd44d9184265a1624cda104eeb5

Review concern 04 of 05: Button align end to end (`button-align`)

Part: Button align

The why:

A centred label is the common case for a button and there was no way to get one ([#105](source:s3)).

The what:

`VitaButtonProps.align` flows through the reconciler as argument 22 of `nativeCreateButton`/`nativeUpdateButton` into `ButtonProps.align`, and `render_button_instance` places the label with `text_layout_line_x` and centres it on `text_line_height`.

Look for:
- Breaking (visual): every existing Button label moves from top-left padding 8 to centred, because the reconciler passes `"center"` when `align` is unset.
- Subtle: a label wider than the button minus 16px padding now overflows on both sides when centred, where it used to overflow only on the right.
- `BUTTON_ARGC` goes from 22 to 23, so an older runtime bundle calling with 22 arguments makes the native create/update a no-op; the runtime and host ship together, so this only bites a stale `js/dist`.

Hunk refs for this concern:
- js/packages/sdk/src/vitadeck-host-types.ts@72+76
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@1+1
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@81+81
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@217+217
- js/packages/runtime/src/vitadeck-react-reconciler-mutation.ts@339+340
- js/packages/runtime/src/runtime-native.d.ts
- src/jslib/instance.c
- src/ui/instance_tree.h
- src/ui/render.c
- js/packages/sdk/README.md