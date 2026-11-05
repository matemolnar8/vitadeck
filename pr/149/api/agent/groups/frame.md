Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 77464ae1945fca65d55985db4fc3f4aa3bc38bff` and `git rev-parse --verify 3f548c17eb2121cd55faa11bab32ac0c8b499483` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 77464ae1945fca65d55985db4fc3f4aa3bc38bff 3f548c17eb2121cd55faa11bab32ac0c8b499483 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  77464ae1945fca65d55985db4fc3f4aa3bc38bff

head               3f548c17eb2121cd55faa11bab32ac0c8b499483

Named refs at pin: 77464ae1945fca65d55985db4fc3f4aa3bc38bff ... 3f548c17eb2121cd55faa11bab32ac0c8b499483

Read the diff:

git diff --find-renames 77464ae1945fca65d55985db4fc3f4aa3bc38bff 3f548c17eb2121cd55faa11bab32ac0c8b499483

Review concern 01 of 02: Glyph upload every frame, and the bootstrap screens (`frame`)

The why:

Glyphs only reached the GPU inside `render_draw_list`, which runs only while a Deck App is ready, so Shell text drawn without one would never appear. The startup and failure screens were [in scope](source:s1) too.

The what:

`bootstrap_draw_deck_canvas` now starts the text frame and uploads dirty atlas pages before anything draws, and the splash, loading line and failure screen draw through `text_draw`, with failure lines wrapped by `text_cluster_width` instead of per-byte `MeasureText`.

Attention (tradeoff):
Glyphs the Shell or the failure screen cache while drawing upload at the top of the next frame, so a first-seen string shows one frame late. With a ready Deck App, `text_begin_frame` now runs twice per frame (here and in `render_draw_list`), which keeps `src/ui/render.c` untouched; nothing caches glyphs between the two calls.

Hunk refs for this concern:
- src/core/bootstrap.c
- tests/fixtures/golden/Linux/14-failure-screen.png