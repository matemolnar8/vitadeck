Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 05cd3b580fb6c9be2272de4548816decc5f3de40` and `git rev-parse --verify 5fbdbe10b4830a6e343e9abe46f88aad89d5284a` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 5fbdbe10b4830a6e343e9abe46f88aad89d5284a -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  05cd3b580fb6c9be2272de4548816decc5f3de40

head               5fbdbe10b4830a6e343e9abe46f88aad89d5284a

Named refs at pin: 05cd3b580fb6c9be2272de4548816decc5f3de40 ... t3/emoji-pack

Read the diff:

git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 5fbdbe10b4830a6e343e9abe46f88aad89d5284a

Review concern 02 of 04: The emoji loader reads entries from the zip (`reader`)

The why:

Without a reader, the zip is just a file the host cannot draw from.

The what:

`emoji_pack_open` indexes the zip's central directory into an stb_ds string map, `load_emoji_image` in `fonts.c` opens the pack on the first emoji and inflates only the named entry, and `glyph_cache_harness` walks every pack entry through the same path.

Attention (tradeoff):
`emoji_pack_read` accepts only deflated entries (method 8), because that is all `cmake -E tar --format=zip` writes. A stored entry from another zip tool reads as a missing emoji and draws the fallback glyph. The pack `FILE *` stays open from the first emoji until `font_registry_shutdown`.

Depends on:
- 01 Configure packs the Twemoji PNGs into one zip (`pack`)

Hunk refs for this concern:
- src/ui/emoji_pack.c
- src/ui/emoji_pack.h
- src/ui/fonts.c
- tests/glyph_cache_harness.c
- CMakeLists.txt@168+168