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

Review concern 04 of 07: `playSound` and `stopSound` in the SDK (`sdk`)

Part: Deck App Sounds

The why:

The author-facing surface the [request](source:s2) names: the SDK plays audio files from assets.

The what:

`sound.ts` wraps the native functions with `SoundName`-typed `playSound(name, { loop })` and `stopSound(name)`, which `index.tsx` exports and the docs describe.

Depends on:
- 03 Sound registry and the native play and stop functions (`native`)

Hunk refs for this concern:
- js/packages/sdk/src/sound.ts
- js/packages/sdk/src/index.tsx
- js/packages/sdk/README.md
- CONTEXT.md
- .agents/skills/vitadeck-deck-app-init/SKILL.md