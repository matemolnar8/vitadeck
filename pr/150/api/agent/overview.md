Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995` and `git rev-parse --verify b0a30605c0c10bd3393bb263b9ce286e06f29813` in this repository.
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

base (merge-base)  72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995

head               b0a30605c0c10bd3393bb263b9ce286e06f29813

Named refs at pin: 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 ... t3/retro-nav-docs

Read the diff:

git diff --find-renames 72e2e3ce9d7c99bb8015c05d4fea7d41a9a69995 b0a30605c0c10bd3393bb263b9ce286e06f29813

Commits:
- b0a30605 docs(skills): call gui-drive.sh from out/, where the testing steps run
- 60f178d1 docs: move the review rules into CODING_STANDARDS.md
- 3647cff5 docs: Ubuntu 26.04 setup and trying the SDK before it is on npm
- f0d05d34 docs(skills): drive the host window without computerUse; move device lessons into the repo
- 94a4dd4b fix(pages): keep a PR's evidence/ folder when republishing its review
- c63e0375 fix(scripts): smoke-docker hands all its build output back, also on failure
- a101f26c test: share the demo section count between the smoke and input harnesses
- 7c5ef35d build: compile the stb_ds implementation once in vitadeck_core

Sources:
- pr PR #150 Fixes the retro's top six findings: GUI driving docs, device lessons, Docker ownership, demo section count, Pages evidence, stb_ds.
  https://github.com/matemolnar8/vitadeck/pull/150
- transcript Claude Code retro session · Nov 5 Retro of 10 sessions across ubuntu-pc and the MacBook Pro; the user asked to fix items 1-6.

The title:

Stop agents rediscovering how to drive the host window, and fix four repo traps

The why:

A retro of recent agent sessions found agents lose time on stale or missing know-how rather than on finding code ([PR #150](source:s1), [retro session](source:s2)).

The what (medium):

Adds `scripts/gui-drive.sh` and rewrites the manual-testing skill around it, moves device lessons from agent memory into the repo docs, fixes four traps (root-owned Docker output, a demo section count kept in three places, Pages republish deleting PR evidence, a per-harness stb_ds implementation), corrects the Linux setup notes, documents trying unreleased packages, and moves the review rules into `CODING_STANDARDS.md`.

## Review concerns

### 01 gui-drive.sh and the manual-testing skill (`gui-drive`)

The skill's run, drive and recording steps now point at `scripts/gui-drive.sh`, which wraps xdotool and ffmpeg on Linux and a compiled Swift CGEvent helper on macOS.

Attention (tradeoff):
On macOS every `key` and `click` activates the vitadeck app with `activateIgnoringOtherApps`, taking focus from whatever the user is doing. Without it, CGEvent keys reach the frontmost app instead. The macOS path compiled and ran on the MacBook Pro, but keys and clicks there are untested.

[groups/gui-drive.md](groups/gui-drive.md)

### 02 FTP and Vita3K lessons in the repo (`device-lessons`)

Footgun 6 in `AGENTS.md` becomes the single statement of the FTP rule, the build skill points at it, and the emulator skill gains the headless, `active-package.txt` and `config.yml` facts.

[groups/device-lessons.md](groups/device-lessons.md)

### 03 smoke-docker.sh restores ownership on exit (`docker-ownership`)

The chown moves into `restore_ownership`, runs from an EXIT trap set at the top of `run_inside`, and now also covers `js/packages/*/dist` and `js/examples/*/.vitadeck`.

[groups/docker-ownership.md](groups/docker-ownership.md)

### 04 Shared demo section count (`demo-sections`)

`ROUNDS` in `deck_app_input_harness.c` comes from `DEMO_SECTION_COUNT`, `smoke_harness.c` static-asserts its `PAGES` length against it, and a comment above `SECTIONS` in the demo names every file that changes with a section.

[groups/demo-sections.md](groups/demo-sections.md)

### 05 Republish keeps evidence/ (`pages-evidence`)

`publishPagesReview` clears every entry of the dest folder except `EVIDENCE_DIR` before copying the new export in, and the pages skill and `AGENTS.md` say where screenshots go.

Attention (tradeoff):
Old evidence files are never removed by a republish: a screenshot dropped from the PR body stays hosted until the PR closes and the workflow prunes `pr/<n>/`. A new export's own `evidence/` merges over the kept one, replacing files with the same name.

[groups/pages-evidence.md](groups/pages-evidence.md)

### 06 stb_ds implementation in vitadeck_core (`stb-ds`)

`src/core/stb_ds.c` joins `VITADECK_CORE_SOURCES` and holds the only `STB_DS_IMPLEMENTATION`, so `main.c`, the harnesses and the fixture comment drop theirs.

[groups/stb-ds.md](groups/stb-ds.md)

### 07 Setup notes and the pre-release recipe (`setup-docs`)

`AGENTS.md` Setup gains the corepack and raylib-from-source notes and points at the new "Try the packages before they are on npm" section of `docs/releasing.md`.

[groups/setup-docs.md](groups/setup-docs.md)

### 08 CODING_STANDARDS.md (`standards`)

The "Hit every surface" rules move from `AGENTS.md` into `CODING_STANDARDS.md` with new ADR, manifest and glossary checks, and `AGENTS.md` keeps a one-paragraph pointer.

Attention (tradeoff):
Implementing agents no longer see the surface rules in their always-loaded context, only the pointer naming them. Reviews now carry those rules, so a miss surfaces in review rather than during implementation.

[groups/standards.md](groups/standards.md)

### 09 Harnesses drop the define and the unused include (`mirrors-1`)

The same two-line removal of `#define STB_DS_IMPLEMENTATION` and `#include "stb_ds.h"` repeats; read one and skip the rest.

Depends on:
- 06 stb_ds implementation in vitadeck_core (`stb-ds`)

[groups/mirrors-1.md](groups/mirrors-1.md)

### 10 Harnesses drop the define (`mirrors-2`)

The same one-line removal of `#define STB_DS_IMPLEMENTATION` repeats; read one and skip the rest.

Depends on:
- 06 stb_ds implementation in vitadeck_core (`stb-ds`)

[groups/mirrors-2.md](groups/mirrors-2.md)

### 11 The implementation moves out of main.c (`mechanical`)

A pure move of the two implementation lines from `src/main.c` into `src/core/stb_ds.c`; safe to skip.

Depends on:
- 06 stb_ds implementation in vitadeck_core (`stb-ds`)

[groups/mechanical.md](groups/mechanical.md)