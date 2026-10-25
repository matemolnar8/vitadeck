Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5` and `git rev-parse --verify 1ac414061e4d8ecdbe718effe4daca0449592e05` in this repository.
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

base (merge-base)  18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5

head               1ac414061e4d8ecdbe718effe4daca0449592e05

Named refs at pin: 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 ... feat/demo-deck-app

Read the diff:

git diff --find-renames 18cbdfd92000926c9c4fd53c3e0f22a076f8b6d5 1ac414061e4d8ecdbe718effe4daca0449592e05

Commits:
- 1ac4140 docs(examples): README for the demo Deck App and a link from the root README
- c6da78b feat(examples): demo Deck App touring every SDK feature

Sources:
- transcript Roadmap thread brief · Oct 25 Build js/examples/demo: one section per SDK feature, Stack/Scroll layout, README, screenshots, no large binaries.

The title:

feat(examples): demo Deck App touring every SDK feature

The why:

Máté asked to [top everything off with a Demo app showcasing all the VitaDeck features](source:s1).

The what (medium):

Adds `js/examples/demo`, a Deck App named VitaDeck Demo with one section per SDK feature, a nav column and L/R switching, plus a README and a link from the root README.

Look for:
- The brief asks to confirm the demo [stages into the VPK's installed library like the others](source:s1); the VPK bundles no example Deck Apps at all (only `runtime.js`, fonts and emoji), so the Vita build was checked for success only.
- The brief names the package `demo`; the directory is `js/examples/demo` and the npm name follows the other examples (`@vitadeck/example-demo`), while the Deck App Package Name the CLI derives from "VitaDeck Demo" is `vitadeck-demo.vdapp`.
- No `CMakeLists.txt` or `pnpm-workspace.yaml` change: both enumerate examples by glob (`examples/*`, `scripts/stage-example-apps.sh`).

## Review concerns

### 01 Example package and declared assets (`package`)

`vitadeck.config.json` declares the `mono` font and the `logo` image that the package files point at.

[groups/package.md](groups/package.md)

### 02 App frame and layout helpers (`frame`)

`App.tsx` renders the active entry of `SECTIONS` inside a header, nav column and footer built from `Stack`s, and every section uses `Label`/`Section` from `ui.tsx`.

Depends on:
- 01 Example package and declared assets (`package`)

[groups/frame.md](groups/frame.md)

### 03 Drawing and layout sections (`visual`)

Five stateless sections draw primitives and nest `Stack`s; `ScrollSection` holds its `Scroll` in a same-size `Rect` because a `Stack` does not lay out a `Scroll`.

Depends on:
- 02 App frame and layout helpers (`frame`)

[groups/visual.md](groups/visual.md)

### 04 Input sections (`input`)

Button, Pad Buttons and Text Input sections turn focus, press and pad events into on-screen state, and the Text Input writes its confirmed value to `storage`.

Depends on:
- 02 App frame and layout helpers (`frame`)

[groups/input.md](groups/input.md)

### 05 Runtime API sections (`runtime`)

Storage, Timers, fetch and Failure Screen sections use the Deck App globals, and `StorageSection` owns the `demo.counter`/`demo.name` keys the Text Input section imports.

Depends on:
- 02 App frame and layout helpers (`frame`)

[groups/runtime.md](groups/runtime.md)

### 06 README (`docs`)

The example README maps each section to its file and explains how to run it on the host and with `vitadeck dev`; the root README links it.

[groups/docs.md](groups/docs.md)