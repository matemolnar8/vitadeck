Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify ecada964bdfdcc6151885f0d2aac72c339057b98` and `git rev-parse --verify c45708bc4894acd44d9184265a1624cda104eeb5` in this repository.
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

base (merge-base)  ecada964bdfdcc6151885f0d2aac72c339057b98

head               c45708bc4894acd44d9184265a1624cda104eeb5

Named refs at pin: ecada964bdfdcc6151885f0d2aac72c339057b98 ... HEAD

Read the diff:

git diff --find-renames ecada964bdfdcc6151885f0d2aac72c339057b98 c45708bc4894acd44d9184265a1624cda104eeb5

Commits:
- c45708b feat(sdk): Button align prop; labels centre vertically and default to centred
- 513b673 fix(sdk): Text defaults to the theme text colour, fontSize 20 and x/y 0
- f36bfdc feat(sdk): export TextAlign, TextWrap, StackAlign, StackDirection and PadButtonPhase

Sources:
- ticket #102 Default Text colour to the theme, x/y to 0, align fontSize with Button, document it.
  https://github.com/matemolnar8/vitadeck/issues/102
- ticket #103 Additive type exports from the SDK index.
  https://github.com/matemolnar8/vitadeck/issues/103
- ticket #105 Add align (default center) and vertical centring; refresh the smoke golden.
  https://github.com/matemolnar8/vitadeck/issues/105
- pr PR #113 One commit per issue.
  https://github.com/matemolnar8/vitadeck/pull/113

The title:

Visible Text defaults, exported prop types, centred Button labels

The why:

Authors building the demo Deck App hit invisible black `Text` on the default theme, could not name `TextAlign`/`StackAlign`/`PadButtonPhase`, and could not centre a Button label ([#102](source:s1), [#103](source:s2), [#105](source:s3)).

The what (small):

Three author-experience fixes: the SDK exports the alignment and phase types its props use, `Text` defaults to the theme text colour at 20px and x/y 0, and `Button` gains `align` with vertically centred, horizontally centred-by-default labels.

Look for:
- [#102](source:s1) asks for `theme.foreground`; the `Theme` type has no such field, so `Text` defaults to `theme.text`.
- [#102](source:s1) asks to note the default change in release notes; the repo has no changelog, so the PR body carries it.

## Review concerns

### 01 Export prop types (`exports`)

The SDK index re-exports `PadButtonPhase` from `pad-buttons` and `StackAlign`, `StackDirection`, `TextAlign`, `TextWrap` from the host prop types.

[groups/exports.md](groups/exports.md)

### 02 Text defaults (`text-defaults`)

The SDK `Text` component fills `color` from `useTheme()`, while the reconciler and the native fallback in `fonts.h` move the size to 20 and the position to 0.

[groups/text-defaults.md](groups/text-defaults.md)

### 03 Examples keep their Text offset (`text-call-sites`)

Each `Text` without `x`/`y` in minesweeper, timers and the scaffold template gains `x={8} y={8}`; smoke already passes every prop, so its golden does not move for this part.

Depends on:
- 02 Text defaults (`text-defaults`)

[groups/text-call-sites.md](groups/text-call-sites.md)

### 04 Button align end to end (`button-align`)

`VitaButtonProps.align` flows through the reconciler as argument 22 of `nativeCreateButton`/`nativeUpdateButton` into `ButtonProps.align`, and `render_button_instance` places the label with `text_layout_line_x` and centres it on `text_line_height`.

[groups/button-align.md](groups/button-align.md)

### 05 Demo lists stay left-aligned; golden refresh (`button-call-sites`)

The demo nav and Scroll-section Buttons pass `align="left"`, and the Linux golden differs from the old one only inside the two smoke Button labels.

Depends on:
- 04 Button align end to end (`button-align`)

[groups/button-call-sites.md](groups/button-call-sites.md)