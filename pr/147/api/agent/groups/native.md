Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 05cd3b580fb6c9be2272de4548816decc5f3de40` and `git rev-parse --verify 92759426382c6cafaf057cbe8556ffe04ec30dd4` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 92759426382c6cafaf057cbe8556ffe04ec30dd4 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  05cd3b580fb6c9be2272de4548816decc5f3de40

head               92759426382c6cafaf057cbe8556ffe04ec30dd4

Named refs at pin: 05cd3b58 ... 92759426

Read the diff:

git diff --find-renames 05cd3b580fb6c9be2272de4548816decc5f3de40 92759426382c6cafaf057cbe8556ffe04ec30dd4

Review concern 03 of 06: Sound registry and the native play and stop functions (`native`)

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