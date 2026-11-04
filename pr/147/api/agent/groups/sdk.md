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

Review concern 04 of 06: `playSound` and `stopSound` in the SDK (`sdk`)

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