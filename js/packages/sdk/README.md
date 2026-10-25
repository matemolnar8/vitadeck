# @vitadeck/sdk

Build tooling and the VitaDeck Runtime API for Deck Apps.

## Requirements

- Node.js 22+
- React **18.3.x** (peer dependency)

## Install

```bash
npm install @vitadeck/sdk
```

pnpm and Bun work the same (`pnpm add`, `bun add`).

## Scaffold a project

```bash
npx @vitadeck/sdk create <project-name>
```

`create` refuses a directory that already has files in it; pass `--force` to scaffold into it anyway. `npx @vitadeck/sdk --help` lists every command and flag.

## Project configuration

`vitadeck.config.json` at the project root:

```json
{
  "name": "My Deck App",
  "entry": "./src/App.tsx",
  "outDir": "./dist",
  "version": "1.2.0",
  "fonts": { "title": "./assets/Title.ttf" },
  "images": { "logo": "./assets/logo.png" }
}
```

- `name` (required): display name shown in the VitaDeck Shell. The package folder name is a slug of it (`My Deck App` → `my-deck-app.vdapp`).
- `entry` (required): the source file whose default export is your root component.
- `outDir` (required): parent folder for the built `.vdapp` folder and `.vdapp.zip`.
- `version` (optional): semver written to the package manifest. Defaults to `version` in `package.json`.
- `fonts` / `images` (optional): name → file path, relative to the project and inside it. Names start with a letter and use letters, digits, `_` or `-` (max 63 characters); `default` is reserved for fonts. Up to 32 of each. Fonts: `.ttf`, `.otf`, `.fnt`, `.bdf`. Images: `.png`, `.jpg`, `.jpeg`, `.bmp`, `.tga`, `.gif`, `.psd`, `.hdr`, `.pic`, `.qoi`. The build copies them into the package and fails if a file is missing.

Reference assets by name, never by path: `<Text font="title" …>`, `<Image image="logo" …>`.

### TypeScript

Extend the SDK preset and include the generated types:

```json
{
  "extends": "@vitadeck/sdk/tsconfig",
  "include": ["src", ".vitadeck/**/*.d.ts"]
}
```

Add `/// <reference types="@vitadeck/sdk/deck-app-env" />` in a `.d.ts` file (the scaffold uses `src/vitadeck-env.d.ts`) to type the globals VitaDeck provides: timers, `getTime`, `fetch`, and `console`.

Each build writes `.vitadeck/font-names.d.ts` and `.vitadeck/image-names.d.ts`, which add your declared names to `VitaDeckFontMap` and `VitaDeckImageMap`. After that, `font` and `image` props only accept declared names. Before the first build, `font` only accepts `"default"` and `image` accepts any string, so build once after adding assets. The folder is regenerated on every build; you can gitignore it.

## Build

From the root of a Deck App (with `vitadeck.config.json`):

```bash
npx @vitadeck/sdk build
```

`npx @vitadeck/sdk` with no subcommand runs **build**. Use `npx @vitadeck/sdk build --no-zip` to skip the `.zip` archive. Builds bundle your app with `NODE_ENV=production`; pass `--dev` (`npx @vitadeck/sdk build --dev`) for a development build.

## Develop on a Vita

Open the VitaDeck Shell on the Vita (or the desktop host), confirm **Upload**, and copy the Runtime Upload URL it shows. Then, from the Deck App root:

```bash
npx @vitadeck/sdk dev http://192.168.1.50:8787/
```

`dev` builds like `build --dev`, uploads the `.vdapp.zip`, then rebuilds and uploads again every time you save. The scheme and port are optional (`192.168.1.50` means `http://192.168.1.50:8787/`). Each upload prints one line:

```text
[14:02:11] installed my-deck-app.vdapp 1.2.0
[14:02:40] 422 invalid_package: manifest.json is missing "entry"
```

Activate the Deck App once in the Shell. After that, each upload replaces the Active Deck App and restarts it with fresh state (a restart, not hot reload). Build errors are printed and watching continues; Ctrl-C stops. "Nothing is listening" means Upload is not open in the Shell or the address is wrong.

## Focus and pad buttons

One `Button` at a time has focus: the D-pad moves it and Cross presses it. Touch presses a `Button` directly.

```tsx
import { Button, usePadButton } from "@vitadeck/sdk";

usePadButton("circle", { onPressStart: () => setScreen("menu") });

<Button label="Play" autoFocus focusColor={theme.accent} onFocus={() => setHint("Start a game")} … />;
```

- `onFocus` / `onBlur` fire when the `Button` gains or loses focus. `focusColor` fills it while focused (default: `color` lightened). `autoFocus` focuses it on mount; the last one mounted wins.
- `usePadButton(button, { onPressStart, onPressEnd })` listens to `"circle"`, `"triangle"`, `"square"`, `"l"` or `"r"` while the component is mounted. Start and Select belong to VitaDeck.
- On the desktop build, arrow keys and Enter are the D-pad and Cross; Backspace, T, S, L and R are Circle, Triangle, Square, L and R; hovering a `Button` with the mouse focuses it. The full table is under [Host mappings](https://github.com/matemolnar8/vitadeck/blob/main/CONTEXT.md#host-mappings).

## Links

- **Repository:** [https://github.com/matemolnar8/vitadeck](https://github.com/matemolnar8/vitadeck) (monorepo; this package is under `js/packages/sdk`.)
- **Issues:** [https://github.com/matemolnar8/vitadeck/issues](https://github.com/matemolnar8/vitadeck/issues)
