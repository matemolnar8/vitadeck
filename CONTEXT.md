# VitaDeck

VitaDeck runs React-authored Deck Apps on a PlayStation Vita. This is the ubiquitous language: use these names in code, docs, UI, and conversation; the _Avoid_ lines list the words they replace.

## Deck Apps and packages

**Deck App**: A user-provided interactive experience rendered by the VitaDeck runtime. It owns the full **Render Surface**.
_Avoid_: Plugin, applet, experience, deck

**Deck App Source Entry**: The author's React entry file; it default-exports one **Deck App Component** and the **VitaDeck SDK** bundles it at build time.
_Avoid_: Plugin file, component file

**Deck App Component**: The default-exported React component of a **Deck App**. It renders **Screen**; it never calls registration APIs.
_Avoid_: Render function, bootstrap

**Deck App Manifest**: `vitadeck.config.json` in a Deck App project: `name`, `entry`, `outDir`, optional `version`, `fonts`, `images`. `outDir` is the parent of the generated `.vdapp` directory.
_Avoid_: Package file, plugin config

**Deck App Package**: The built, portable unit the SDK produces and VitaDeck consumes. It holds compiled Deck App code and assets, not the runtime or React.
_Avoid_: Full runtime bundle, plugin package, upload blob

**Deck App Package Directory**: The unpacked form of a **Deck App Package**; its name is the **Deck App Package Name** plus the `.vdapp` suffix.
_Avoid_: Archive, bundle folder, dist folder

**Deck App Package Name**: The SDK-generated filesystem-safe name of a package. It identifies a package in the **Installed Deck App Library** and keys **Deck App Storage**.
_Avoid_: Output directory, package path, app title

**Deck App Package Manifest**: `manifest.json` inside the package, validated natively in `src/core/package_manifest.c` and emitted by the SDK CLI. Carries the **Deck App Package Schema Version**, the **Deck App Package Version**, the display name, the entry path, and declared fonts and images.
_Avoid_: Source manifest, npm package metadata, config file

**Deck App Package Schema Version**: The `schemaVersion` field that versions the manifest shape VitaDeck reads. Bump it when the shape changes.
_Avoid_: App release version, package version, semver

**Deck App Package Version**: The user-visible semver `version` field, taken from the project's `package.json` unless the **Deck App Manifest** overrides it.
_Avoid_: Schema version, SDK version, VitaDeck version

**Deck App Package Entry**: The generated JavaScript file in a package that hands the compiled **Deck App Component** to the runtime through the **Package Registration Hook**.
_Avoid_: User entry, source entry, main runtime

**Package Registration Hook**: The internal global convention between a **Deck App Package Entry** and the **VitaDeck Runtime Bundle**. Not an author API, not a security boundary.
_Avoid_: Public SDK API, security boundary, author API

**Deck App Font**: A font file shipped in a package and declared in the manifest; author code references it by declared name. Missing glyphs in a TrueType or OpenType font fall back to the **Runtime Default Font**, then a **Runtime Fallback Font**; `.fnt` and `.bdf` fonts never fall back and draw no emoji. Emoji render as the bundled Twemoji images, not from any font.
_Avoid_: Raw font path, system font, hardcoded font

**Deck App Image**: An image file shipped in a package and declared in the manifest; author code references it by declared name.
_Avoid_: Raw image path, texture path, sprite sheet path

**Runtime Default Font**: VitaDeck's bundled DejaVu Sans, used when no font is requested and as the first fallback. A build that could not download it uses raylib's built-in font.
_Avoid_: Built-in Raylib font, implicit app font, serif default

**Runtime Fallback Font**: One of VitaDeck's bundled fonts for characters no earlier font has: Droid Sans Fallback Full for Chinese ideographs and Japanese kana, then Noto Sans KR for Hangul. Each loads the first time a character from its Unicode blocks is drawn; a character no font has draws its Twemoji image when one exists, otherwise the missing-glyph box.
_Avoid_: System font, CJK default font

**Font Size**: Text height in raw pixels, as requested by author code.
_Avoid_: Text scale, named text size, point size

**Deck App Storage**: The per-**Deck App Package Name** string key-value store (`storage` in the **VitaDeck Runtime API**, 1 MiB) that survives restarts. It is the only filesystem a Deck App reaches; **Runtime Upload Replacement** keeps it, **Installed Deck App Removal** deletes it.
_Avoid_: localStorage, save file, cache, settings

## Running Deck Apps

**Installed Deck App Library**: The on-device set of **Deck App Packages**, persisted until **Installed Deck App Removal**. **Runtime Upload** publishes into it.
_Avoid_: Upload cache, package folder, app store

**Active Deck App**: The package currently running. Chosen in the **VitaDeck Shell** and persisted; when none is set, VitaDeck activates the first library entry at startup or the first **Runtime Upload** publish.
_Avoid_: Current plugin, selected component

**Installed Deck App Removal**: Deleting a package from the library via the Shell. Cannot target the **Active Deck App**.
_Avoid_: Uninstalling VitaDeck, clearing cache, revoking permissions

**Deck App Runtime Restart**: A fresh JavaScript context, triggered by changing the **Active Deck App** or by **Runtime Upload Replacement** of it.
_Avoid_: Hot reload, refresh, soft reload

**Deck App Failure Screen**: The VitaDeck-painted screen that replaces a **Deck App** after an uncaught error (boot, render, effects, handlers, timers, or an unhandled promise rejection), showing the message, the top of the stack, and that **Start Input** opens the Shell. A **Deck App Runtime Restart** clears it.
_Avoid_: Crash screen, red box, blank screen

**Render Surface**: The full Vita display, owned by the running **Deck App**.
_Avoid_: Page, route, viewport

## Input

**System Input**: A Vita input reserved for VitaDeck itself: **Start Input**, and Select, which does nothing yet.
_Avoid_: App shortcut, global hotkey, hidden button

**Start Input**: Toggles between the **Deck App** and the **VitaDeck Shell**. Hiding the Shell requires an **Active Deck App**; it never stops the **Runtime Upload Listener**.
_Avoid_: Game menu, global pause, app back button

**Focus**: The one **Deck App** control the D-pad moves between and Cross presses. **Button** and **Text Input** take Focus; a touch press presses directly and clears it. On the host, mouse hover and mouse press move Focus to that **Button**.
_Avoid_: Selection, hover, highlight, active element

**Press**: The input gesture behind **Button** and **Text Input** callbacks.
_Avoid_: Click, mouse down, mouse up, hover

**Pad Button**: A Vita button a **Deck App** reads directly through `usePadButton`: Circle, Triangle, Square, L or R. Delivered only while the Shell is hidden; one held when the Shell hides is ignored until released.
_Avoid_: Hotkey, shortcut, raw button, gamepad event

**Shell Face Input**: Face-button conventions inside the **VitaDeck Shell**: Cross confirms, Circle backs out (cancels a pending removal, otherwise hides the Shell). A connected gamepad works alongside the host keyboard.
_Avoid_: Deck App **Press**, mouse click, tap

## Runtime API

**VitaDeck Runtime API**: The public, author-facing API for rendering UI and reaching VitaDeck capabilities. Native bridge entry points are not part of it.
_Avoid_: Native bindings, intrinsic elements, host config

**Runtime Theme**: The VitaDeck-provided palette exposed through the **VitaDeck Runtime API**; VitaDeck owns the provider.
_Avoid_: Sample theme, app-local theme

**Screen**: The root component, representing the full Vita display.
_Avoid_: Surface, viewport, canvas

**Button**: The component that receives **Press** (`onPress`, `onPressStart`, `onPressEnd`) and **Focus** (`onFocus`, `onBlur`, `focusColor`, `autoFocus`).
_Avoid_: Pressable rect, clickable shape

**Text Input**: The `TextInput` component for one line of text. A **Press** starts an edit, in the Vita keyboard dialog or in place on the host; `onChange` fires once with the final text on confirm, nothing on cancel. No other Deck App input is delivered during an edit; opening the Shell cancels it.
_Avoid_: Text field, text box, IME, keyboard input

**Image**: A visual-only component drawing a **Deck App Image**. It takes no input.
_Avoid_: Texture, sprite, bitmap widget

**Stack**: A visual-only component placing children in a column or row with gap, padding and cross-axis alignment, sized to content unless given a size. `Scroll` is a column Stack whose content scrolls in a fixed viewport; it is the only component taking scroll input.
_Avoid_: Flexbox, flex container, layout box, list

## Shell

**VitaDeck Shell**: VitaDeck's management UI for choosing and managing installed packages, opened by **System Input**, never by a Deck App.
_Avoid_: Deck App menu, overlay app, launcher screen

**Shell Home Screen**: The default Shell view: the **Installed Deck App Library** sorted by manifest `name` (case-insensitive, then **Deck App Package Name**), each with its version, plus the **Shell Upload Entry**.
_Avoid_: Live Area, system settings, account screen

**Shell Upload Entry**: The Upload row at the end of the **Shell Home Screen**. Confirming it starts the **Runtime Upload Listener** and the row shows the **Runtime Upload URL** (or the bind error, with no URL) while the rest of the Shell stays usable. Confirming it again is **Shell Upload Cancel**: it stops the listener without closing the Shell. There is no other cancel shortcut.
_Avoid_: Hidden gesture, Deck App **Press**, browser bookmark, FTP client

## Runtime Upload

**Runtime Upload**: Installing a **Runtime Upload Archive** over the LAN into the **Installed Deck App Library**. It does not activate the package unless none is active.
_Avoid_: Hot reload, live edit

**Runtime Upload Listener**: The VitaDeck-owned HTTP server (`src/upload/`) that serves the **Runtime Upload Web UI** and accepts uploads. Binds on all interfaces at port 8787, falling forward up to 10 ports; if none binds, it does not run. It runs from **Shell Upload Entry** until **Shell Upload Cancel** or exit, through Shell hides, active app changes, and both successful and failed uploads.
_Avoid_: Public API, always-on LAN service, background upload daemon

**Runtime Upload URL**: The `http://IP:PORT/` address of the Web UI, using a LAN-reachable IP and the port actually bound.
_Avoid_: FTP URL, deep link, marketplace URL

**Runtime Upload Web UI**: The browser page at `GET /` for picking and uploading an archive, showing the JSON result.
_Avoid_: Deck App UI, store frontend, CDN-hosted site

**Runtime Upload POST**: `POST /upload`, taking one archive as `multipart/form-data` (field `archive`) or `application/zip`, answering JSON: `ok: true` with `packageName` and `version`, or `ok: false` with `code` and `message` and a status reflecting the error class. One ingest at a time; a concurrent one gets 409. Other paths 404, wrong methods on `/upload` 405; there is no other endpoint.
_Avoid_: GraphQL mutation, WebSocket upload, chunked multi-file APIs

**Runtime Upload Limits**: Caps on archive size, unpacked size and entry count (16 MB, 64 MB, 256 entries). Over-limit uploads install nothing.
_Avoid_: User quota, app permissions, download budget

**Runtime Upload Archive**: The `.vdapp.zip` the SDK emits next to each package: a zip holding exactly one top-level `.vdapp` directory and nothing else.
_Avoid_: Source zip, VPK, npm tarball

**Runtime Upload Staging**: The temporary location where an archive is unpacked and validated before publish; an invalid package is never visible as installed.
_Avoid_: Cache, download folder, build output

**Runtime Upload Replacement**: An upload whose **Deck App Package Name** matches an installed package overwrites it in place. If it is the **Active Deck App**, a **Deck App Runtime Restart** follows.
_Avoid_: Uninstall, rollback, package rename

**Upload Pairing**: A possible future ephemeral credential for the listener. Initial Runtime Upload trusts the LAN.
_Avoid_: Login, account, DRM

## SDK and runtime

**VitaDeck SDK**: `@vitadeck/sdk` on npm: the **VitaDeck Runtime API** plus the `vitadeck` CLI that builds packages and scaffolds projects.
_Avoid_: Runtime package, CLI package, app framework

**Deck App Project Scaffold**: The CLI-generated standalone project, pinning `@vitadeck/sdk` with a caret range at the generating SDK version.
_Avoid_: Example copy, starter blob, template folder

**VitaDeck Runtime Bundle**: `runtime.js`, the VitaDeck-owned bundle the host embeds; it bootstraps React and the reconciler and loads the active package.
_Avoid_: App bundle, package entry, main app

**Runtime Dependency**: A JavaScript dependency the **VitaDeck Runtime Bundle** provides instead of packages bundling it. React is one.
_Avoid_: App dependency, bundled dependency, peer package

**Deck App Workspace**: The pnpm workspace at `js/` holding the SDK, the runtime, and the example Deck Apps.
_Avoid_: Plugin folder, examples folder

## Host Control

**Host Control**: A **Deck App** asking a nearby LAN computer to run a typed command through the **Host Companion**. Results use the same `ok` / `code` / `message` JSON shape as **Runtime Upload POST**.
_Avoid_: Remote desktop, PC control, Runtime Upload

**Host Companion**: The program on a nearby Mac or Windows computer that performs **Host Control Link** and runs commands. Not the desktop build of VitaDeck.
_Avoid_: Host Control Companion, desktop app, daemon, host build

**Host Control Link**: The Companion, given the address shown in the Shell, tells VitaDeck where to send commands; VitaDeck keeps that address until replaced or forgotten. Addresses only, no credentials; it does not use the **Runtime Upload Listener**.
_Avoid_: Pairing, Upload Pairing, session, typing the computer's address on the Vita

## Host mappings

Every Vita input has a desktop host equivalent, and a connected gamepad works alongside the keyboard. `src/ui/input.c` implements the **Deck App** column, `src/platform/text_input_host.c` the **Text Input** editing rows, and `src/shell/shell.c` the Shell column; change them together with this table.

| Vita | Desktop host | In a **Deck App** | In the **VitaDeck Shell** |
| --- | --- | --- | --- |
| D-pad | Arrow keys | Moves **Focus** | Moves between rows and actions |
| Cross | Enter | Presses the **Button** or **Text Input** with **Focus** | Confirm |
| Circle | Backspace | **Pad Button** `circle` | Back |
| Triangle | T | **Pad Button** `triangle` | |
| Square | S | **Pad Button** `square` | |
| L | L | **Pad Button** `l` | |
| R | R | **Pad Button** `r` | |
| Start | F1 | **Start Input**: opens the Shell | **Start Input**: hides the Shell |
| Select | none | Reserved **System Input** | Reserved **System Input** |
| Touch press | Mouse press | Presses the **Button** under it; touch clears **Focus**, the mouse focuses that **Button** | |
| | Mouse hover | Moves **Focus** to the hovered **Button** | |
| Touch drag, left stick | Mouse wheel | Scrolls a `Scroll`: drag and wheel the one under the pointer, the stick the one holding **Focus** | |
| Keyboard dialog keys | Typing, Backspace | While a **Text Input** edit runs: edit its text | |
| Keyboard dialog Enter | Enter | While a **Text Input** edit runs: confirms it | |
| Keyboard dialog close or cancel | Escape | While a **Text Input** edit runs: cancels it | |
