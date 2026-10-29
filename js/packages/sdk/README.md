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
- `entry`: the source file whose default export is your root component. Required unless the project only builds a Host Plugin (see [Standalone plugins](#standalone-plugins)).
- `outDir` (required): parent folder for the built `.vdapp` folder and `.vdapp.zip`.
- `version` (optional): semver written to the package manifest. Defaults to `version` in `package.json`.
- `fonts` / `images` (optional): name → file path, relative to the project and inside it. Names start with a letter and use letters, digits, `_` or `-` (max 63 characters); `default` is reserved for fonts. Up to 32 of each. Fonts: `.ttf`, `.otf`, `.fnt`, `.bdf`. Images: `.png`, `.jpg`, `.jpeg`, `.bmp`, `.tga`, `.gif`, `.psd`, `.hdr`, `.pic`, `.qoi`. The build copies them into the package and fails if a file is missing.
- `hostPlugin` (optional): the Deck App's own Host Control Plugin. `entry` is its implementation (`.ts`, `.mts`, `.js` or `.mjs`); `files` (optional) is a folder copied into the package as `files/`. Both are relative to the project. The build writes the `<outDir>/<slug>.vdplugin` package and `<slug>.vdplugin.zip`; see [Your own Host Plugin](#your-own-host-plugin).

Reference assets by name, never by path: `<Text font="title" …>`, `<Button font="title" fontSize={24} …>`, `<Image image="logo" …>`.

Text, Button and TextInput take any `fontSize` in pixels, and text is UTF-8. Without `font` they use the runtime's DejaVu Sans. Characters a `.ttf` or `.otf` font lacks render in DejaVu Sans, then in Droid Sans Fallback Full (Chinese and Japanese) or Noto Sans KR (Korean); a character none of them has renders as a box. `.fnt` and `.bdf` fonts have no fallback.

Emoji render in colour, sized to `fontSize`, from the [Twemoji](https://github.com/jdecked/twemoji) images (CC-BY 4.0) that ship with VitaDeck. Single emoji, emoji with `U+FE0F`, skin tones, flags and keycaps work. A ZWJ sequence Twemoji has no image for draws as its first emoji, and a symbol such as `❤` without `U+FE0F` stays a font glyph. `.fnt` and `.bdf` fonts draw no emoji.

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

## Storage

`storage` keeps string key-value pairs for your Deck App across restarts. Store anything richer with `JSON.stringify`.

```tsx
import { storage } from "@vitadeck/sdk";

const [best, setBest] = useState(() => Number(storage.getItem("best") ?? 0));
function finish(score: number) {
  if (score <= best) return;
  setBest(score);
  storage.setItem("best", String(score));
}
```

`getItem(key)` returns `null` for a missing key. `setItem` and `removeItem` return once the change is on disk. Each Deck App gets 1 MiB; a `setItem` that would go past it throws a `RangeError` and keeps the previous value. Reinstalling the Deck App over Runtime Upload keeps its storage; removing it in the VitaDeck Shell deletes it.

## Host Control

`@vitadeck/sdk/host-control` lets a Deck App call a computer on the same LAN that runs the [VitaDeck Host Companion](https://github.com/matemolnar8/vitadeck/tree/main/js/packages/host-companion) (`vitadeck-host-companion`). The user links computers in the VitaDeck Shell (Host Control); a Deck App never needs their addresses.

```tsx
import { createHostClient, systemStatsPlugin } from "@vitadeck/sdk/host-control";

const stats = await createHostClient(systemStatsPlugin).read();
if (stats.ok) console.log(`${stats.cpuPercent}% CPU`);
else if (stats.code === "not_linked") setHint("Link a computer in the VitaDeck Shell");
```

- Commands come in **Host Control Plugins**. Every companion has the built-in `core` (`capabilities`, `echo`; also exported as `host`) and `systemStats` (`read`) plugins.
- `createHostClient(plugin, { hostId })` returns a typed client for one plugin on one computer; leave out `hostId` for the most recent link. `linkedHosts()` lists the linked computers (`id`, `name`, `url`), newest first, and reflects Shell changes on its next call.
- Every method resolves to `{ ok: true, ...result }` or `{ ok: false, code, message }` and never rejects, so TypeScript makes you check `ok`. Besides the companion's codes, the client reports `not_linked`, `unreachable`, `timeout` (5 s by default; the command may still have run) and `bad_response`. A failed command is never retried.
- `capabilities()` lists the plugins a computer loaded, so a Deck App can tell an `unknown_plugin` apart before calling it.

### Events from the computer

A plugin can also send **events** to a Deck App as soon as something happens on the computer, instead of the Deck App polling for it. `watchHostEvents` keeps one request waiting at the companion, which answers it the moment an event is emitted, or after 25 s with nothing, when it simply waits again:

```tsx
import { corePlugin, watchHostEvents } from "@vitadeck/sdk/host-control";

useEffect(
  () =>
    watchHostEvents(corePlugin, (event) => setNotice(event.data.text), { hostId, onFailure: (f) => setHint(f.code) }),
  [hostId],
);
```

- It returns a function that stops watching, so it fits a `useEffect` cleanup. A failed wait is retried after 5 s, or 30 s after a `timeout`.
- `core` emits `notice` for `vitadeck-host-companion notify "Build done"`, so any script on the computer can reach Deck Apps with no plugin of its own.
- Events are best effort: a companion restart or more than 100 events between two waits drops some.

### Your own Host Plugin

When the built-in plugins are not enough, write a plugin: in the Deck App's own project, or on its own for several Deck Apps to use (see [Standalone plugins](#standalone-plugins)). `create --host-plugin` scaffolds a Deck App with one:

```bash
npx @vitadeck/sdk create my-app --host-plugin
```

- `host/plugin.ts` defines the commands with `defineHostPlugin(id, { name: command<Payload, Result>(guard) })`. Deck Apps import it, so it is bundled into them and must not import Node.js modules. The companion runs `guard` on every payload before your code.
- `host/companion.ts` default-exports `implementHostPlugin(plugin, { name: async (payload) => result })`. It runs in the companion with Node.js. `tsc` fails if a command is missing or returns the wrong type; throw `new HostCommandError("unsupported_platform", "...")` to answer with a specific code.
- `"hostPlugin": { "entry": "./host/companion.ts" }` in `vitadeck.config.json` makes `build` and `dev` also write `dist/my-app.vdplugin`, a **Host Plugin Package**, and its `dist/my-app.vdplugin.zip`. The package holds `manifest.json` and `companion.mjs`, your implementation bundled with its npm dependencies (pure JavaScript only). Add `"files": "./host/files"` to ship other files too, like scripts or data; read them with `new URL("./files/<name>", import.meta.url)` when a command runs, since a rebuild that changes only `files/` does not re-run your module. The `.vdplugin.zip` keeps no file permissions, so run a shipped script through its interpreter (`execFile("sh", [path])`) rather than directly.
- To send events, declare them as a third argument, `defineHostPlugin("lights", { toggle: ... }, { changed: event<{ room: string; on: boolean }>() })`, and emit them from a lifecycle in `host/companion.ts`: `implementHostPlugin(lightsPlugin, { toggle: ... }, { start: (emit) => { const stop = watchLights((room, on) => emit("changed", { room, on })); return stop; } })`. `start` runs when the companion loads the plugin, and the function it returns runs when it unloads or reloads. `emit` only accepts the declared events and their data.
- `pnpm host` builds and runs a companion with your plugin, reloading it whenever `vitadeck dev` rebuilds. Link it once with `npx vitadeck-host-companion link <address>`.

Users install your plugin with `vitadeck-host-companion install my-app.vdplugin.zip`, so ship the `.vdplugin.zip` next to the `.vdapp.zip`. When they have not, your Deck App gets `unknown_plugin`; tell them what to install.

### Standalone plugins

A plugin does not need a Deck App. `create --host-plugin-only` scaffolds a project with only `host/`, and a `vitadeck.config.json` with `hostPlugin` but no `entry`:

```bash
npx @vitadeck/sdk create lights --host-plugin-only
```

`build` writes only `dist/lights.vdplugin` and `dist/lights.vdplugin.zip`, and `dev` rebuilds them on every save without a Runtime Upload URL; `pnpm host` runs them in a companion. The project's `package.json` exports `host/plugin.ts`, so a Deck App adds the project as a dependency (a local path such as `pnpm add ../lights`, a git URL, or npm once you remove `"private": true`) and imports the commands. Installed from git or npm, the plugin uses the Deck App's `@vitadeck/sdk`, its peer dependency; a linked local path uses the plugin's own copy, which only costs a few bytes in the bundle.

```tsx
import { createHostClient } from "@vitadeck/sdk/host-control";
import { hostPlugin as lights } from "lights";

const result = await createHostClient(lights).greet({ name: "Vita" });
```

Its users install `lights.vdplugin.zip` once, and every Deck App that depends on it can call it.

This subpath is bundled into your Deck App, along with any plugin registry you import.

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

## Text

`<Text>` draws in the theme's `text` colour at `fontSize` 20, at `x`/`y` 0 inside its parent, unless you pass `color`, `fontSize`, `x` or `y`. Give it a `width` to `align` it (`"left"`, `"center"`, `"right"`) or to `wrap="word"`.

A `Button` label is centred vertically and, unless `align` says `"left"` or `"right"`, horizontally; `fontSize` defaults to 20 there too.

## Layout

`Stack` places its children one after another, so you don't compute every `x` and `y` yourself.

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
- A `Stack` without `color` takes no hits itself: a tap between its children reaches whatever is behind it. With `color` it behaves like a `Rect`.
- `x` and `y` default to 0. On a child in a `Stack`, the one along the flow is a margin before the child and the other an offset from its aligned position.
- `Text` flows in a `Stack` like any child, as wide as its widest line (or its `width`) and one `lineHeight` per line tall. In a `Rect`, each `Text` instead sits one line below its `y` for every `Text` before it.
- `Scroll` lays out its children as a column `Stack` and scrolls them with touch drag, the left stick or the mouse wheel.
- A `Scroll` flows in a `Stack` like any child. Give it a `height` in a column (`width` in a row), or make it the last child and leave that out: it fills the rest of a `Stack` that has one.

```tsx
<Stack x={20} y={20} width={920} height={504} gap={12}>
  <Rect width={920} height={60} color={theme.surface} />
  <Scroll width={920} gap={8} padding={10}>
    {rows}
  </Scroll>
</Stack>
```

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

`maxLength` (default 256, at most 2048) counts like `string.length`. It takes focus like a `Button`, with `focusColor` as its outline (default: `textColor`).

## Links

- **Repository:** [https://github.com/matemolnar8/vitadeck](https://github.com/matemolnar8/vitadeck) (monorepo; this package is under `js/packages/sdk`.)
- **Issues:** [https://github.com/matemolnar8/vitadeck/issues](https://github.com/matemolnar8/vitadeck/issues)
