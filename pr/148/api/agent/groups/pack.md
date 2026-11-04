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

Review concern 01 of 04: Configure packs the Twemoji PNGs into one zip (`pack`)

The why:

The zip has to exist before anything can read it; this group produces it with CMake alone.

The what:

`twemoji.cmake` passes the new `PACK twemoji.zip` option, and `vitadeck_fetch_asset` lands the stripped PNG names in a staging folder, then runs `cmake -E tar --format=zip` over them.

Attention (contract):
With `PACK`, the stamp name gains a `-twemoji.zip` suffix, so an existing checkout's `Twemoji-<sha>` stamp no longer matches. Reconfigure then downloads again, deletes the loose PNGs the old stamp lists, and writes the zip. An offline reconfigure during that upgrade builds without emoji until the next online one.

Hunk refs for this concern:
- cmake/fetch_asset.cmake
- cmake/twemoji.cmake