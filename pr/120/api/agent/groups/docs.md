Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 02d88881c71ebf1091332268ecbcd587dddafb01` and `git rev-parse --verify 538d2833a7caafed2a5042946bfa378f95553eb0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 538d2833a7caafed2a5042946bfa378f95553eb0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  02d88881c71ebf1091332268ecbcd587dddafb01

head               538d2833a7caafed2a5042946bfa378f95553eb0

Named refs at pin: 02d88881c71ebf1091332268ecbcd587dddafb01 ... HEAD

Read the diff:

git diff --find-renames 02d88881c71ebf1091332268ecbcd587dddafb01 538d2833a7caafed2a5042946bfa378f95553eb0

Review concern 03 of 04: Docs point at the new goldens (`docs`)

Part: Demo goldens

The why:

`AGENTS.md` asks to fix docs and skills in place when a change makes them inaccurate.

The what:

ADR 0002, `AGENTS.md` and the `vitadeck-smoke` skill now name `tests/fixtures/golden/Linux/` and the demo instead of `smoke_golden.Linux.png` and the smoke app; wording only.

Depends on:
- 01 Harness steps through the demo (`harness`)

Hunk refs for this concern:
- docs/adr/0002-smoke-ci-linux-xvfb.md
- AGENTS.md
- .agents/skills/vitadeck-smoke/SKILL.md