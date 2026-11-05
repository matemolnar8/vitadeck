Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 77464ae1945fca65d55985db4fc3f4aa3bc38bff` and `git rev-parse --verify 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 77464ae1945fca65d55985db4fc3f4aa3bc38bff 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  77464ae1945fca65d55985db4fc3f4aa3bc38bff

head               2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Named refs at pin: 77464ae1945fca65d55985db4fc3f4aa3bc38bff ... 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Read the diff:

git diff --find-renames 77464ae1945fca65d55985db4fc3f4aa3bc38bff 2d3ee2e2d2f41b108a9b7a049a5df48bfc53fcb3

Review concern 02 of 03: Shell draws through text_draw (`shell`)

The why:

This is the [asked change](source:s1): Shell text through `text_draw` with the Runtime Default Font, layout kept equivalent.

The what:

Three static helpers in `shell.c` (`shell_text`, `shell_text_width`, `shell_text_y`) replace every `DrawText`/`MeasureText` call one to one, and `shell_text_y` centres a line in a row by `text_line_height` rather than the font size.

Depends on:
- 01 Glyph upload every frame, and the bootstrap screens (`frame`)

Hunk refs for this concern:
- src/shell/shell.c
- tests/shell_input_harness.c