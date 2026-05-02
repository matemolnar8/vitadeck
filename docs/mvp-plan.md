# VitaDeck Deck App Iteration 1 Plan

## Goal

Enable VitaDeck to bundle one selected Deck App at build time and render it as the full VitaDeck experience. This is the first iteration toward runtime-uploaded Deck Apps, not the final package model.

## Iteration 1 Choices

- The user-provided runnable unit is a **Deck App**.
- Iteration 1 Deck Apps are provided as source, not runtime uploads.
- A Deck App is selected at build time and owns the full Vita render surface.
- Deck App authors use the **VitaDeck Runtime API**, not raw host elements.
- Runtime UI components use generic names from `@vitadeck/runtime`: `Screen`, `Rect`, `Text`, and `Button`.
- `Button` is the only public interactable component for iteration 1.
- Public button callbacks use press vocabulary: `onPress`, `onPressStart`, and `onPressEnd`.
- Internal input naming should be aligned away from mouse terminology where it crosses runtime boundaries.
- The public root component is `Screen`.
- Deck Apps must explicitly render `Screen`.
- Theme is provided by VitaDeck runtime bootstrap; Deck Apps only consume it.
- A Deck App source entry default-exports a React component; VitaDeck owns bootstrapping and rendering.
- Each Deck App has a tiny `vitadeck.json` manifest with `name` and `entry`.
- `js/vitadeck.config.json` selects the active Deck App manifest for normal builds.
- Runtime upload over an HTTP server on the Vita is a future goal, not an MVP requirement.
- Iteration 1 Deck Apps live in-repo under `js/deck-apps/`.
- The existing demo pages become four separate sample Deck Apps instead of one built-in navigation app.

## Proposed Skeleton

- `js/vitadeck.config.json`
- `js/deck-apps/hello/vitadeck.json`
- `js/deck-apps/hello/App.tsx`
- `js/deck-apps/counters/vitadeck.json`
- `js/deck-apps/counters/App.tsx`
- `js/deck-apps/timers/vitadeck.json`
- `js/deck-apps/timers/App.tsx`
- `js/deck-apps/minesweeper/vitadeck.json`
- `js/deck-apps/minesweeper/App.tsx`
- `js/src/runtime/index.tsx`
- `js/src/runtime/theme.tsx`
- `js/src/main.tsx` imports `@vitadeck/active-deck-app`
- `js/rolldown.config.js` reads the project config and manifest, then aliases `@vitadeck/active-deck-app` to the selected Deck App entry.

## Not In MVP

- Runtime upload
- Host computer actions or macro execution
- Desktop companion app
- Multiple installed Deck Apps
- App picker or built-in shell
- Manifest permissions
- Arbitrary Deck App npm dependencies
- Icons, settings screens, or version metadata
- Layout engine
- Storage, networking, or device APIs for Deck Apps

## Acceptance Criteria

- The active Deck App is typechecked by `pnpm --dir js tsc`.
- The active Deck App is bundled by `pnpm --dir js build`.
- The native build packages the bundled active Deck App without changing the existing C runtime loading path.
- Each sample Deck App can be selected through `js/vitadeck.config.json` and built independently.
