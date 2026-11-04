Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5fbdbe10b4830a6e343e9abe46f88aad89d5284a` and `git rev-parse --verify 4968a00c087f538de9251d93c083192cee84bddf` in this repository.
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

base (merge-base)  5fbdbe10b4830a6e343e9abe46f88aad89d5284a

head               4968a00c087f538de9251d93c083192cee84bddf

Named refs at pin: origin/t3/emoji-pack ... t3/audio-playback

Read the diff:

git diff --find-renames 5fbdbe10b4830a6e343e9abe46f88aad89d5284a 4968a00c087f538de9251d93c083192cee84bddf

Commits:
- 4968a00c docs: warn that probing the Vita with nc crashes it
- ba91e892 fix: count the Sound section in the demo benchmark and input harness
- 6a1e22c5 feat: Deck App Sounds with playSound and stopSound
- 258a40a5 build: vendor miniaudio dev-0.12 for its Vita audio backend

Sources:
- pr PR #147 Sounds declared in vitadeck.config.json, played by name on host and Vita via miniaudio dev-0.12; demo Sound section; host PipeWire capture as proof.
  https://github.com/matemolnar8/vitadeck/pull/147
- transcript Claude Code session · Nov 4 Add audio support so the SDK can play audio files from assets, add a demo section, use miniaudio.h after checking PS Vita support.
- commit docs: warn that probing the Vita with nc crashes it

The title:

Deck Apps play sounds on the desktop and the Vita

The why:

Máté asked that [the SDK can play audio files from assets](source:s2), with a demo section, using miniaudio if it works on the PS Vita.

The what (large):

Deck Apps declare `sounds` like images and play them with `playSound` and `stopSound`, through a vendored miniaudio `dev-0.12` on the host and the Vita, and the demo gets a Sound section. It also carries a docs-only warning against probing the Vita with `nc`.

Look for:
- The request asks for [miniaudio.h, checked for PS Vita support](source:s2). Released miniaudio has no Vita backend, so `vendor/miniaudio` is a submodule pinned to the unreleased `dev-0.12` branch, not a released single header; ADR 0006 gives the reason.

## Review concerns

### 01 miniaudio dev-0.12 as a submodule and its own library (`vendor`)

`.gitmodules` adds the shallow `vendor/miniaudio` submodule, and `CMakeLists.txt` builds it as a `-w` static library linked next to `vitadeck_core`, with the docs recording the pin.

Attention (tradeoff):
The pin `245bbb8` sits on an unreleased branch, so its API and Vita backend can change under a future bump. In exchange VitaDeck ships no audio code of its own under `src/platform/`. ADR 0006 says to move to a 0.12 release and retest on the Vita when the pin moves.

[groups/vendor.md](groups/vendor.md)

### 02 `sounds` in the package contract, C and TypeScript (`contract`)

The CLI copies `sounds` to `sounds/` and types their names in `sound-names.d.ts`, and `package_manifest.c` accepts the same field, each through one shared asset routine instead of per-kind copies.

Attention (contract):
`schemaVersion` stays 1 although the manifest gains `sounds`. An older host skips the unknown key and loads the package, but `playSound` is then missing from its runtime SDK and throws. A package without sounds is byte-for-byte the same shape as before.

[groups/contract.md](groups/contract.md)

### 03 Sound registry and the native play and stop functions (`native`)

`bootstrap.c` calls `sound_registry_load_package` beside the font and image registries, and `jslib/sound.c` exposes `nativePlaySound` and `nativeStopSound` over the registry in `src/audio/sounds.c`.

Attention (risk):
The JS thread reads `g_package_sounds` without a lock. That is safe only because `src/main.c` stops the JS thread before `bootstrap_reload_active_package_fonts`, the one caller that rewrites the registry. A future caller that reloads assets with the JS thread running would race.

Depends on:
- 01 miniaudio dev-0.12 as a submodule and its own library (`vendor`)
- 02 `sounds` in the package contract, C and TypeScript (`contract`)

[groups/native.md](groups/native.md)

### 04 `playSound` and `stopSound` in the SDK (`sdk`)

`sound.ts` wraps the native functions with `SoundName`-typed `playSound(name, { loop })` and `stopSound(name)`, which `index.tsx` exports and the docs describe.

Depends on:
- 03 Sound registry and the native play and stop functions (`native`)

[groups/sdk.md](groups/sdk.md)

### 05 Demo Sound section and the tests that count sections (`demo`)

`SoundSection` plays the chime and toggles the MP3 loop, and inserting it after Image renumbers the smoke pages and goldens, adds an input-harness round and one `r` to `demo-scroll`.

Attention (tradeoff):
`demo-scroll` still measures the Scroll section, now with a 13-button nav list. The Vita3K and Vita baselines in `tests/bench/baselines/` were not re-recorded, so the next comparison against them includes that small extra render cost.

Depends on:
- 04 `playSound` and `stopSound` in the SDK (`sdk`)

[groups/demo.md](groups/demo.md)

### 06 Failure Screen golden renumbered (`mechanical`)

A rename with a near-identical image; skip it unless the Failure Screen looks wrong in the demo group's goldens.

Depends on:
- 05 Demo Sound section and the tests that count sections (`demo`)

[groups/mechanical.md](groups/mechanical.md)

### 07 Warning against probing the Vita (`probe-docs`)

A new AGENTS.md footgun and matching lines in three skills tell agents to ask for the Vita IP and upload with curl instead of probing ports.

[groups/probe-docs.md](groups/probe-docs.md)