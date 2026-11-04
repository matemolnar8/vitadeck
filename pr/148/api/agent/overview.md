Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 05cd3b580fb6c9be2272de4548816decc5f3de40` and `git rev-parse --verify 5fbdbe10b4830a6e343e9abe46f88aad89d5284a` in this repository.
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

base (merge-base)  05cd3b580fb6c9be2272de4548816decc5f3de40

head               5fbdbe10b4830a6e343e9abe46f88aad89d5284a

Named refs at pin: 05cd3b580fb6c9be2272de4548816decc5f3de40 ... t3/emoji-pack

Read the diff:

git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 5fbdbe10b4830a6e343e9abe46f88aad89d5284a

Commits:
- 5fbdbe10 feat(ui): ship the Twemoji images as one zip

Sources:
- pr PR #148 One zip instead of ~4000 PNGs; VPK goes from 4021 to 13 files; rendering unchanged.
  https://github.com/matemolnar8/vitadeck/pull/148

The title:

Ship emoji as one zip so the VPK installs and uploads fast

The why:

Installing the VPK and uploading the emoji were slow because [the Vita's filesystem and VitaShell's FTP handle many small files badly](source:s1).

The what (medium):

Configure packs the 4009 Twemoji PNGs into `assets/emoji/twemoji.zip`, and the emoji loader reads single entries from that zip instead of opening one file per emoji.

## Review concerns

### 01 Configure packs the Twemoji PNGs into one zip (`pack`)

`twemoji.cmake` passes the new `PACK twemoji.zip` option, and `vitadeck_fetch_asset` lands the stripped PNG names in a staging folder, then runs `cmake -E tar --format=zip` over them.

Attention (contract):
With `PACK`, the stamp name gains a `-twemoji.zip` suffix, so an existing checkout's `Twemoji-<sha>` stamp no longer matches. Reconfigure then downloads again, deletes the loose PNGs the old stamp lists, and writes the zip. An offline reconfigure during that upgrade builds without emoji until the next online one.

[groups/pack.md](groups/pack.md)

### 02 The emoji loader reads entries from the zip (`reader`)

`emoji_pack_open` indexes the zip's central directory into an stb_ds string map, `load_emoji_image` in `fonts.c` opens the pack on the first emoji and inflates only the named entry, and `glyph_cache_harness` walks every pack entry through the same path.

Attention (tradeoff):
`emoji_pack_read` accepts only deflated entries (method 8), because that is all `cmake -E tar --format=zip` writes. A stored entry from another zip tool reads as a missing emoji and draws the fallback glyph. The pack `FILE *` stays open from the first emoji until `font_registry_shutdown`.

Depends on:
- 01 Configure packs the Twemoji PNGs into one zip (`pack`)

[groups/reader.md](groups/reader.md)

### 03 Vita uploads send the one zip (`upload`)

`upload_vita` now sends `twemoji.zip` with one `curl -T`, still guarded by the per-IP stamp, and `bench.sh` drops the comment about the hour-long first emoji upload, since its missing-file listing now finds one file.

Depends on:
- 01 Configure packs the Twemoji PNGs into one zip (`pack`)

[groups/upload.md](groups/upload.md)

### 04 Docs name the zip (`docs`)

Wording-only updates that say the emoji ship as `assets/emoji/twemoji.zip`; you can skim them.

[groups/docs.md](groups/docs.md)