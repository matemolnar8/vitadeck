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

Review concern 01 of 07: miniaudio dev-0.12 as a submodule and its own library (`vendor`)

Part: Deck App Sounds

The why:

Only miniaudio's `dev-0.12` branch has a Vita backend, which the [request](source:s2) needs.

The what:

`.gitmodules` adds the shallow `vendor/miniaudio` submodule, and `CMakeLists.txt` builds it as a `-w` static library linked next to `vitadeck_core`, with the docs recording the pin.

Attention (tradeoff):
The pin `245bbb8` sits on an unreleased branch, so its API and Vita backend can change under a future bump. In exchange VitaDeck ships no audio code of its own under `src/platform/`. ADR 0006 says to move to a 0.12 release and retest on the Vita when the pin moves.

Hunk refs for this concern:
- .gitmodules
- vendor/miniaudio
- CMakeLists.txt
- vendor/PATCHES.md
- docs/adr/0006-miniaudio-dev-branch-for-sound.md
- AGENTS.md@23+23