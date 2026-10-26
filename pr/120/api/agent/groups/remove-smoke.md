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

Review concern 02 of 04: Delete the smoke example (`remove-smoke`)

Part: Demo goldens

The why:

The smoke app [is no longer needed](source:s1) once the demo is the fixture.

The what:

The `smoke` example is deleted and its DejaVu Sans Mono moves into `demo/assets/fonts`, replacing the demo's symlinks into it.

Depends on:
- 01 Harness steps through the demo (`harness`)

Hunk refs for this concern:
- js/examples/smoke/assets/fonts/DejaVu-LICENSE.txt
- js/examples/smoke/assets/images/smoke-logo.png
- js/examples/smoke/package.json
- js/examples/smoke/src/App.tsx
- js/examples/smoke/src/vitadeck-env.d.ts
- js/examples/smoke/tsconfig.json
- js/examples/smoke/vitadeck.config.json
- js/examples/demo/assets/fonts/DejaVu-LICENSE.txt
- js/examples/demo/README.md
- tests/layout_harness.c