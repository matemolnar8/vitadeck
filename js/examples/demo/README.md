# VitaDeck Demo

A tour of `@vitadeck/sdk`, one section per feature. L and R (keys L and R on the desktop) step through the sections, or pick one from the list on the left with the D-pad and Cross (arrow keys and Enter, or the mouse).

| Section                                           | Shows                                                                                                    |
| ------------------------------------------------- | -------------------------------------------------------------------------------------------------------- |
| [Rect & theme](src/sections/RectSection.tsx)      | `Screen`, `useTheme`, `Rect` fill and outline variants, `borderRadius`                                   |
| [Text](src/sections/TextSection.tsx)              | `align`, `wrap`, `lineHeight`, `fontSize`, a Deck App Font, Hungarian accents, emoji, CJK                |
| [Image](src/sections/ImageSection.tsx)            | A Deck App Image sized by width, height, or both                                                         |
| [Button](src/sections/ButtonSection.tsx)          | `onPress`, `onPressStart`/`onPressEnd`, `focusColor`, `autoFocus`, `onFocus`/`onBlur`, `font`/`fontSize` |
| [Pad Buttons](src/sections/PadButtonSection.tsx)  | `usePadButton` for Circle, Triangle and Square; [App.tsx](src/App.tsx) uses L and R                      |
| [Stack](src/sections/StackSection.tsx)            | Column and row, `gap`, `padding`, `align`, nesting                                                       |
| [Scroll](src/sections/ScrollSection.tsx)          | A 40-row list with momentum inside a column `Stack`                                                      |
| [Text Input](src/sections/TextInputSection.tsx)   | A controlled `TextInput` whose confirmed value is saved                                                  |
| [Storage](src/sections/StorageSection.tsx)        | `storage.getItem`/`setItem`/`removeItem` for a counter, with a reset                                     |
| [Timers](src/sections/TimersSection.tsx)          | A `setInterval` clock and a `setTimeout` toast                                                           |
| [fetch](src/sections/FetchSection.tsx)            | Loading, result and error states for a GitHub API request                                                |
| [Failure Screen](src/sections/FailureSection.tsx) | A Button that throws, showing the Deck App Failure Screen                                                |

`Label` and `Section` in [src/ui.tsx](src/ui.tsx) wrap `Text` in a sized `Rect`, because a `Stack` lays out `Rect`s but not `Text`.

The smoke test screenshots every section against the goldens in [tests/fixtures/golden](../../../tests/fixtures/golden), so a visual change here needs `scripts/smoke-docker.sh update-golden`.

## Run it

On the desktop host, from the repository root:

```bash
cmake --preset default && ninja -C out run
```

`run` stages every example into `out/vitadeck-data/installed-deck-apps`; open the VitaDeck Shell with F1 and activate VitaDeck Demo.

On a Vita (or the desktop host), open the Shell and point `vitadeck dev` at the URL under **Upload and link**:

```bash
cd js/examples/demo
pnpm exec vitadeck dev 192.168.1.50
```

Every save rebuilds and uploads the Deck App, then restarts it.
