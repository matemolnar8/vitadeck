Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 07001bbdac339a0767b626376cda96d8530a1d37` and `git rev-parse --verify 600ca4ec1f763b6662f205fe8cb068bba809c1c1` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 07001bbdac339a0767b626376cda96d8530a1d37 600ca4ec1f763b6662f205fe8cb068bba809c1c1 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  07001bbdac339a0767b626376cda96d8530a1d37

head               600ca4ec1f763b6662f205fe8cb068bba809c1c1

Named refs at pin: 07001bbdac339a0767b626376cda96d8530a1d37 ... HEAD

Read the diff:

git diff --find-renames 07001bbdac339a0767b626376cda96d8530a1d37 600ca4ec1f763b6662f205fe8cb068bba809c1c1

Review concern 03 of 03: Docs stop calling the companion unpublished (`docs`)

The why:

AGENTS.md, the companion README, ADR 0004 and two skills said the companion is not on npm, which this change makes untrue.

The what:

Each doc drops its "not on npm yet" note, ADR 0004 moves Publishing from Deferred into the lockstep decision, and the companion README switches to absolute links that work on npmjs.com.

Depends on:
- 01 Publishable companion package (`package`)

Hunk refs for this concern:
- AGENTS.md
- docs/adr/0004-host-control-sliced-delivery.md
- js/packages/host-companion/README.md
- .agents/skills/vitadeck-host-control/SKILL.md
- .agents/skills/vitadeck-deck-app-init/SKILL.md