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

## Layout

`Stack` places its `Rect`, `Button`, `TextInput`, `Image` and `Stack` children one after another, so you don't compute every `x` and `y` yourself.

```tsx
import { Button, Image, Stack } from "@vitadeck/sdk";

<Stack x={20} y={20} width={920} gap={12} padding={8} align="center">
  <Image image="logo" height={80} />
  <Stack direction="row" gap={8}>
    <Button label="Easy" width={200} height={56} … />
    <Button label="Hard" width={200} height={56} … />
  </Stack>
</Stack>;
```

- `direction` is `"column"` (default) or `"row"`; `gap` and `padding` are pixels; `align` (`"start"`, `"center"`, `"end"`) places children across the other axis.
- Without `width` or `height` a `Stack` fits its content. `color` fills it.
- `x` and `y` default to 0. On a child in a `Stack`, the one along the flow is a margin before the child and the other an offset from its aligned position.
- `Text` is not laid out: wrap it in a `Rect`. A `Scroll` inside a `Stack` keeps its own `x`/`y` too.
- `Scroll` lays out its children as a column `Stack` and scrolls them with touch drag, the left stick or the mouse wheel.

## Focus and pad buttons

One `Button` or `TextInput` at a time has focus: the D-pad moves it and Cross presses it. Touch presses it directly.

```tsx
import { Button, usePadButton } from "@vitadeck/sdk";

usePadButton("circle", { onPressStart: () => setScreen("menu") });

<Button label="Play" autoFocus focusColor={theme.accent} onFocus={() => setHint("Start a game")} … />;
```

- `onFocus` / `onBlur` fire when the `Button` gains or loses focus. `focusColor` fills it while focused (default: `color` lightened). `autoFocus` focuses it on mount; the last one mounted wins.
- `usePadButton(button, { onPressStart, onPressEnd })` listens to `"circle"`, `"triangle"`, `"square"`, `"l"` or `"r"` while the component is mounted. Start and Select belong to VitaDeck.
- On the desktop build, arrow keys and Enter are the D-pad and Cross; Backspace, T, S, L and R are Circle, Triangle, Square, L and R; hovering a `Button` with the mouse focuses it. The full table is under [Host mappings](https://github.com/matemolnar8/vitadeck/blob/main/CONTEXT.md#host-mappings).

## Text input

`TextInput` is a controlled single-line field. Pressing it starts an edit: the Vita opens its keyboard dialog, pre-filled with `value` and titled with `placeholder`; the desktop build edits in place, where you type, Backspace deletes, Enter confirms and Escape cancels. `onChange` fires once with the final text when the edit is confirmed; a cancelled edit fires nothing.

```tsx
import { TextInput } from "@vitadeck/sdk";

const [name, setName] = useState("");

<TextInput x={20} y={80} width={400} height={56} value={name} onChange={setName} placeholder="Your name" />;
```

`maxLength` (default 256, at most 2048) counts like `string.length`. It takes focus like a `Button`, with `focusColor` as its outline (default: `textColor`). Text is UTF-8; characters outside the runtime font render as boxes.

## Links

- **Repository:** [https://github.com/matemolnar8/vitadeck](https://github.com/matemolnar8/vitadeck) (monorepo; this package is under `js/packages/sdk`.)
- **Issues:** [https://github.com/matemolnar8/vitadeck/issues](https://github.com/matemolnar8/vitadeck/issues)
