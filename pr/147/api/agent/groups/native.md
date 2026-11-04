Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5fbdbe10b4830a6e343e9abe46f88aad89d5284a` and `git rev-parse --verify 4968a00c087f538de9251d93c083192cee84bddf` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 5fbdbe10b4830a6e343e9abe46f88aad89d5284a 4968a00c087f538de9251d93c083192cee84bddf -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  5fbdbe10b4830a6e343e9abe46f88aad89d5284a

head               4968a00c087f538de9251d93c083192cee84bddf

Named refs at pin: origin/t3/emoji-pack ... t3/audio-playback

Read the diff:

git diff --find-renames 5fbdbe10b4830a6e343e9abe46f88aad89d5284a 4968a00c087f538de9251d93c083192cee84bddf

Review concern 03 of 07: Sound registry and the native play and stop functions (`native`)

Part: Deck App Sounds

The why:

Playback itself: the registry loads a package's sounds with the asset reload and the JS thread plays them by name.

The what:

`bootstrap.c` calls `sound_registry_load_package` beside the font and image registries, and `jslib/sound.c` exposes `nativePlaySound` and `nativeStopSound` over the registry in `src/audio/sounds.c`.

Attention (risk):
The JS thread reads `g_package_sounds` without a lock. That is safe only because `src/main.c` stops the JS thread before `bootstrap_reload_active_package_fonts`, the one caller that rewrites the registry. A future caller that reloads assets with the JS thread running would race.

Depends on:
- 01 miniaudio dev-0.12 as a submodule and its own library (`vendor`)
- 02 `sounds` in the package contract, C and TypeScript (`contract`)

Hunk refs for this concern:
- src/audio/sounds.c
- src/audio/sounds.h
- src/jslib/sound.c
- src/jslib/jslib.c
- src/jslib/jslib_internal.h
- src/core/bootstrap.c
- js/packages/runtime/src/runtime-native.d.ts