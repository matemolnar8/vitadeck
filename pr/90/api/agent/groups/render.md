Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a67144241bd9ac86b8052502902eddcfb74cbd88` and `git rev-parse --verify b3e88a9a19239e8f491616bfc853578a055ce6f4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a67144241bd9ac86b8052502902eddcfb74cbd88 b3e88a9a19239e8f491616bfc853578a055ce6f4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a67144241bd9ac86b8052502902eddcfb74cbd88

head               b3e88a9a19239e8f491616bfc853578a055ce6f4

Named refs at pin: a67144241bd9ac86b8052502902eddcfb74cbd88 ... HEAD

Read the diff:

git diff --find-renames a67144241bd9ac86b8052502902eddcfb74cbd88 b3e88a9a19239e8f491616bfc853578a055ce6f4

Review concern 03 of 05: Text drawing through the cache (`render`)

The why:

Text, Button and TextInput must use the cache for fallback and exact-size glyphs ([commit](source:s2)).

The what:

`render.c` measures and draws through `text.c`, which takes `VdFont`s from the reworked `fonts.c` and uploads changed atlas pages once per frame after `cache_glyphs` warms them.

Look for:
- Perf: `cache_glyphs` walks the whole tree every frame and measures each Text, Button label, TextInput value, placeholder and edit text once more before drawing.
- Startup no longer bakes a 64 px atlas: `font_registry_init` only reads DejaVu Sans and calls `stbtt_InitFont`.
- Switching packages calls `text_reset` from `unload_package_fonts`, which drops every cached glyph and atlas texture, Runtime Default Font glyphs included.
- Line height now comes from font metrics (ascent + descent + line gap). DejaVu Sans has no line gap, so its lines keep their height; an author font with a line gap gets taller lines.
- The golden changes because glyphs are sharper at exact size and advances are no longer floored at 64 px. Lines are a few pixels wider; sizes and baselines match. The smoke header now draws `Árvíztűrő tükörfúrógép 🎮` in its own `Rect`, because a Rect stacks each Text child one line below the previous one; the emoji draws as DejaVu Sans's `.notdef` box until the colour emoji slice.

Depends on:
- 02 Glyph cache core (`cache`)

Hunk refs for this concern:
- src/ui/text.h
- src/ui/text.c
- src/ui/fonts.h
- src/ui/fonts.c
- src/ui/render.c
- CMakeLists.txt@166+166
- CMakeLists.txt@357+364
- tests/fixtures/smoke_golden.Linux.png
- js/examples/smoke/src/App.tsx