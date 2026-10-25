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

## Links

- **Repository:** [https://github.com/matemolnar8/vitadeck](https://github.com/matemolnar8/vitadeck) (monorepo; this package is under `js/packages/sdk`.)
- **Issues:** [https://github.com/matemolnar8/vitadeck/issues](https://github.com/matemolnar8/vitadeck/issues)
