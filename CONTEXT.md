# VitaDeck

VitaDeck lets people run custom interactive deck-style interfaces on a PlayStation Vita.

## Language

**Deck App**:
A user-provided interactive VitaDeck experience rendered by the VitaDeck runtime.
_Avoid_: Plugin, applet, experience, deck

**Deck App Source Entry**:
The source-level React entry file that is compiled into a **Deck App** bundle.
_Avoid_: Plugin file, component file

**Deck App Component**:
The default-exported React component from a **Deck App Source Entry**.
_Avoid_: Render function, bootstrap

**Deck App Manifest**:
The metadata file that identifies a **Deck App** and its **Deck App Source Entry**.
_Avoid_: Package file, plugin config

**Deck App Package Manifest**:
The metadata file inside a **Deck App Package Directory** that VitaDeck reads before loading the package, including a **Deck App Package Schema Version**.
_Avoid_: Source manifest, npm package metadata, config file

**Deck App Package Schema Version**:
The manifest field that versions the **Deck App Package Manifest** shape read by VitaDeck.
_Avoid_: App release version, package version, semver

**Deck App Package Version**:
The user-visible semver of a **Deck App Package**, stored in `manifest.json` as `version` for display and updates.
_Avoid_: Schema version, SDK version, VitaDeck version

**Deck App Package**:
The built portable unit produced from a **Deck App** project and consumed by VitaDeck.
_Avoid_: Full runtime bundle, plugin package, upload blob

**Deck App Package Directory**:
The unpacked directory form of a **Deck App Package** used by VitaDeck during local development and native packaging.
_Avoid_: Archive, bundle folder, dist folder

**Deck App Package Name**:
The SDK-generated filesystem-safe name for a **Deck App Package Directory**.
_Avoid_: Output directory, package path, app title

**Deck App Package Entry**:
The generated JavaScript file inside a **Deck App Package** that registers the compiled **Deck App Component** with VitaDeck.
_Avoid_: User entry, source entry, main runtime

**Package Registration Hook**:
The internal global convention used by generated **Deck App Package Entries** to hand their compiled component to the **VitaDeck Runtime Bundle**.
_Avoid_: Public SDK API, security boundary, author API

**Active Deck App**:
The **Deck App Package** currently selected to run in VitaDeck.
_Avoid_: Current plugin, selected component

**Installed Deck App Library**:
The on-device collection of **Deck App Packages** available for activation, persisted across VitaDeck restarts until **Installed Deck App Removal**. Packages installed via **Runtime Upload** live here after a successful publish, not only in transient **Runtime Upload Staging**.
_Avoid_: Upload cache, package folder, app store

**VitaDeck Shell**:
The VitaDeck-owned management UI for choosing and managing installed **Deck App Packages**.
_Avoid_: Deck App menu, overlay app, launcher screen

**Shell Face Input**:
VitaDeck **System Input** face-button conventions for the **VitaDeck Shell**: Cross confirms the focused control; Circle backs out or activates **Shell Upload Cancel** when focused.
_Avoid_: Deck App **Press**, mouse click, tap

**Shell Home Screen**:
The default **VitaDeck Shell** view for browsing the **Installed Deck App Library**, performing **Installed Deck App Removal**, and opening **Runtime Upload** through the primary **Shell Upload Entry** action. Library entries are ordered alphabetically by the **`name`** field in each **Deck App Package Manifest** (case-insensitive), with **Deck App Package Name** as the tie-breaker when **`name`** values collide.
_Avoid_: Live Area, system settings, account screen

**Shell Upload Entry**:
The primary **Shell Home Screen** control that opens the **Shell Upload Screen**, using the same **Shell Face Input** focus-and-confirm pattern as other top-level Shell actions; not relegated to a submenu, long-press, or **Deck App** UI.
_Avoid_: Hidden gesture, Deck App **Press**, browser bookmark

**Shell Upload Screen**:
The **VitaDeck Shell** **Runtime Upload** mode that attempts to start the **Runtime Upload Listener**, shows the **Runtime Upload URL** on-device when binding succeeds, shows an on-device bind-failure state with no URL when every port in the fallback range is in use, and stays on this screen until **Shell Upload Cancel** or until a **Runtime Upload** publish succeeds (success returns to **Shell Home Screen** while keeping the **VitaDeck Shell** visible; the **Runtime Upload Listener** may keep running — see **Runtime Upload Listener**). **Shell Upload Cancel** is sufficient to leave upload mode after bind failure; no in-screen retry is required.
_Avoid_: Deck App screen, FTP client, OS browser chrome

**Shell Upload Cancel**:
The explicit **Shell Upload Screen** control that stops the **Runtime Upload Listener** and returns to the **Shell Home Screen** without closing the **VitaDeck Shell** (on-screen only on macOS at first).
_Avoid_: Browser back, global back gesture, undo upload

**Installed Deck App Removal**:
Deleting a **Deck App Package** from the **Installed Deck App Library** using the **VitaDeck Shell**.
_Avoid_: Uninstalling VitaDeck, clearing cache, revoking permissions

**System Input**:
A VitaDeck-reserved input that controls VitaDeck itself instead of the running **Deck App**.
_Avoid_: App shortcut, global hotkey, hidden button

**Start Input**:
The Vita `Start` **System Input** that toggles between the **Deck App** and **VitaDeck Shell** when not on the **Shell Upload Screen**; on the **Shell Upload Screen**, **Start Input** does not leave upload mode — use **Shell Upload Cancel** or finish a **Runtime Upload** publish (success returns to **Shell Home Screen**).
_Avoid_: Game menu, global pause, app back button

**Host Start Mapping**:
The macOS keyboard key that maps to the Vita `Start` **System Input** (F1).
_Avoid_: Escape, window manager shortcuts, in-app hotkeys

**Host Shell Confirm Mapping**:
The macOS keyboard key that maps to Cross confirm in **Shell Face Input** while the **VitaDeck Shell** has focus (Enter).
_Avoid_: Mouse click, Deck App **Press**, text field submission in **Deck Apps**

**Host Shell Back Mapping**:
The macOS keyboard key that maps to Circle back in **Shell Face Input** while the **VitaDeck Shell** has focus (Backspace).
_Avoid_: Browser back, undo typing in **Deck Apps**, delete file shortcuts

**LAN HTTP Listener**:
The VitaDeck-owned HTTP server on the LAN that multiplexes **Runtime Upload** and **Host Control** routes on one bound port for the lifetime of a Vitadeck session. The user does not turn it on or off from the **VitaDeck Shell**; Vitadeck starts it when the session begins and keeps it running until exit, except when **LAN HTTP Listener Recovery** stops and restarts it because the network is unavailable or binding failed.
_Avoid_: Shell upload toggle, per-feature enable switches, separate upload and host ports by default

**LAN HTTP URL**:
The `http://IP:PORT/` address of the bound **LAN HTTP Listener**, shown in the **VitaDeck Shell** when the listener is listening, using a LAN-reachable **IP** and the **PORT** actually bound.
_Avoid_: Host computer URL, FTP URL, deep link

**LAN HTTP Listener Recovery**:
When the **LAN HTTP Listener** cannot stay bound or reachable on the LAN (no network, interface down, or port bind failure), Vitadeck stops or avoids the listener and retries until the network is usable again. This is the only intended stop/start cycle—not a user-facing off switch.
_Avoid_: Manual server toggle, Shell Upload Cancel stopping the listener, disabling Host Control from Shell

**Host Control**:
The Vitadeck capability for **Deck Apps** to ask a nearby computer on the LAN to run actions through a **Host Control Companion**, using **Host Control** routes on the **LAN HTTP Listener**.
_Avoid_: Runtime Upload, USB control, cloud remote desktop

**Host Control Companion**:
The program on the user’s computer that connects to the **LAN HTTP URL** and executes **Host Control** commands on that machine.
_Avoid_: VitaDeck runtime, Deck App Package, browser upload client

**Runtime Upload Listener**:
The **Runtime Upload** routes on the **LAN HTTP Listener** (including the **Runtime Upload Web UI** and **Runtime Upload POST**). Not a separate server process from **Host Control** routes.
_Avoid_: Standalone upload-only daemon, public REST catalog

**Runtime Upload Web UI**:
The LAN web page served by the **Runtime Upload Listener** for choosing, dragging/dropping, and uploading a **Runtime Upload Archive**, showing success and failure results in the browser.
_Avoid_: Deck App UI, store frontend, CDN-hosted site

**Runtime Upload HTTP Contract**:
The **Runtime Upload Listener** author-facing HTTP surface is **`GET /`** for the **Runtime Upload Web UI** plus whatever same-origin static assets that page requires, and **`POST /upload`** for **Runtime Upload POST**. Undocumented paths respond **404**; non-**POST** requests to **`/upload`** respond **405**. There is no separate health, readiness, or version HTTP endpoint in the initial **Runtime Upload** iteration.
_Avoid_: Public REST catalog, mandatory OpenAPI, extension plugin routes

**Runtime Upload URL**:
The **LAN HTTP URL** while **Runtime Upload Enablement** is on, used to reach the **Runtime Upload Web UI** in a browser.
_Avoid_: FTP URL, deep link, marketplace URL, host companion address

**Runtime Upload POST**:
The HTTP `POST /upload` endpoint on the **Runtime Upload Listener** that ingests a **Runtime Upload Archive** as either `multipart/form-data` (browser-primary) or `application/zip` (automation-friendly). For `multipart/form-data`, the file part field name is **`archive`** (single file part per request). Responses are always JSON (`Content-Type: application/json`): on success, `ok: true` plus **`packageName`** (**Deck App Package Name**) and **`version`** (**Deck App Package Version**); on failure, `ok: false` plus a short machine **`code`** and human **`message`**. Typical HTTP statuses include **200** success, **413** over **Runtime Upload Limits**, **400** malformed request body, **415** unsupported `Content-Type`, **422** archive layout or manifest validation failure, and **409** when **Runtime Upload Single-Flight** rejects a concurrent ingest.
_Avoid_: GraphQL mutation, WebSocket upload, chunked multi-file APIs

**Runtime Upload Single-Flight**:
At most one **Runtime Upload** ingest pipeline (from accepting **Runtime Upload POST** through **Runtime Upload Staging** validation to publish or failure) runs at a time; another **Runtime Upload POST** while one is in progress receives **HTTP 409** and JSON failure, with no upload queue.
_Avoid_: Parallel unpack, pipelined installs, last-wins overwrite

**Upload Pairing**:
A future optional ephemeral credential that authorizes clients to use a **Runtime Upload Listener**.
_Avoid_: Login, account, DRM

**Runtime Upload Limits**:
VitaDeck-enforced caps on **Runtime Upload Archive** size, unpacked size, and entry count to prevent zip-style abuse (initially 16MB uploaded, 64MB unpacked, 256 entries).
_Avoid_: User quota, app permissions, download budget

**Runtime Upload Staging**:
A temporary on-disk location where **Runtime Upload** unpacks and validates a **Runtime Upload Archive** before it becomes visible in the **Installed Deck App Library**.
_Avoid_: Cache, download folder, build output

**Runtime Upload Replacement**:
A **Runtime Upload** that overwrites an existing **Installed Deck App** because the uploaded `.vdapp` **Deck App Package Name** matches.
_Avoid_: Uninstall, rollback, package rename

**Deck App Runtime Restart**:
Restarting the Deck App JavaScript runtime with a fresh context after **Runtime Upload Replacement** of the **Active Deck App** or after changing the **Active Deck App**.
_Avoid_: Hot reload, refresh, soft reload

**Runtime Upload Archive**:
A zip file that unpacks to exactly one top-level **Deck App Package Directory** whose name ends in `.vdapp`.
_Avoid_: Source zip, VPK, npm tarball

**Deck App Workspace**:
The in-repository JavaScript workspace containing the **VitaDeck SDK**, VitaDeck runtime project, and example Deck App projects.
_Avoid_: Plugin folder, examples folder

**Runtime Upload**:
A future installation flow where a **Runtime Upload Archive** is uploaded through the **Runtime Upload Web UI** (or compatible HTTP clients) while the **Runtime Upload Listener** is running (opened from **Shell Home Screen**; the listener can stay up after leaving the **Shell Upload Screen** — see **Runtime Upload Listener**). Successful publishes add or replace entries in the persistent **Installed Deck App Library** like other installed packages.
_Avoid_: Hot reload, live edit

**Render Surface**:
The full Vita display area owned by the currently running **Deck App**.
_Avoid_: Page, route, viewport

**VitaDeck Runtime API**:
The public author-facing API used by **Deck Apps** to render UI and access VitaDeck capabilities.
_Avoid_: Native bindings, intrinsic elements, host config

**VitaDeck Runtime Bundle**:
The VitaDeck-owned JavaScript bundle that bootstraps the runtime and loads the selected **Deck App Package**.
_Avoid_: App bundle, package entry, main app

**Runtime Dependency**:
A JavaScript dependency provided by the **VitaDeck Runtime Bundle** rather than bundled into a **Deck App Package**.
_Avoid_: App dependency, bundled dependency, peer package

**VitaDeck SDK**:
The npm package that provides the **VitaDeck Runtime API** and Deck App build tooling, distributed on the public npm registry as `@vitadeck/sdk` so **Deck App** projects can live outside the **Deck App Workspace**.
_Avoid_: Runtime package, CLI package, app framework

**Deck App Project Scaffold**:
The SDK-generated starting project layout for authoring one standalone **Deck App**. Outside the **Deck App Workspace**, the scaffold pins `@vitadeck/sdk` with a **caret range** anchored to the **VitaDeck SDK** version that produced the scaffold.
_Avoid_: Example copy, starter blob, template folder

**Runtime Theme**:
The VitaDeck-provided styling palette available to **Deck Apps** through the **VitaDeck Runtime API**.
_Avoid_: Sample theme, app-local theme

**Screen**:
The root **VitaDeck Runtime API** component representing the full Vita display.
_Avoid_: Surface, viewport, canvas

**Button**:
The MVP **VitaDeck Runtime API** component for user interaction.
_Avoid_: Pressable rect, clickable shape

**Press**:
The VitaDeck input gesture used by public **Button** callbacks.
_Avoid_: Click, mouse down, mouse up, hover

## Relationships

- A **Deck App** is rendered by the VitaDeck runtime.
- A **Deck App** is authored against the **VitaDeck Runtime API**.
- A **Deck App** owns the full **Render Surface** in the MVP.
- A **Deck App Source Entry** is bundled at build time for the MVP.
- A **Deck App Source Entry** default-exports one **Deck App Component**.
- A **Deck App Manifest** contains `name` and `entry` for the MVP.
- A **Deck App Manifest** selects which **Deck App Source Entry** is bundled.
- A **Deck App Package Manifest** includes `schemaVersion` (**Deck App Package Schema Version**), `version` (**Deck App Package Version**), package display name, and package entry path.
- A **Deck App Package** contains compiled **Deck App** code and metadata, but not the VitaDeck runtime.
- A **Deck App Package Directory** is the initial on-disk artifact format for a **Deck App Package**.
- A **Deck App Package Directory** always uses a `.vdapp` suffix.
- A **Deck App Package Name** is generated by the **VitaDeck SDK**, typically from Deck App metadata.
- A **Deck App Package Entry** is generated by the **VitaDeck SDK** from a **Deck App Source Entry**.
- A **Deck App Package Entry** uses the **Package Registration Hook** generated by the **VitaDeck SDK**.
- The **Package Registration Hook** is an internal compatibility convention, not a security boundary.
- Deck App authors export a **Deck App Component**; they do not call VitaDeck registration APIs directly.
- A **Deck App Source Entry** may use normal TypeScript and ES module syntax; the **VitaDeck SDK** compiles it into the **Deck App Package Entry**.
- **Runtime Upload** transports a **Deck App Package Directory** as a **Runtime Upload Archive** without changing the package contents.
- A **Runtime Upload Archive** contains exactly one top-level `.vdapp` directory and no other top-level entries.
- The project configuration selects the **Active Deck App** by pointing at a built **Deck App Package Directory**.
- The **Deck App Workspace** is a pnpm workspace rooted at `js/`.
- The **Deck App Workspace** contains package-shaped example Deck App projects under `js/examples/`.
- The **Deck App Workspace** contains the local **VitaDeck SDK** package under `js/packages/sdk/`.
- The **Deck App Workspace** contains the VitaDeck-owned runtime package under `js/packages/runtime/`.
- VitaDeck owns bootstrapping and renders the **Deck App Component**.
- React is a **Runtime Dependency** owned by the **VitaDeck Runtime Bundle**, not bundled into **Deck App Packages**.
- The **VitaDeck SDK** must make the compatible React version explicit for Deck App development.
- The **VitaDeck SDK** declares React compatibility through peer dependencies.
- The **VitaDeck SDK** build validates that a Deck App project resolves a compatible React version.
- The **VitaDeck SDK** provides a TypeScript config preset for Deck App projects.
- The **VitaDeck SDK** provides a **Deck App Project Scaffold** for creating package-shaped Deck App projects.
- A **Deck App Project Scaffold** created outside the **Deck App Workspace** declares `@vitadeck/sdk` as `^version` matching the scaffolding **VitaDeck SDK** release.
- A **Deck App Project Scaffold** is author-facing and is not a repo migration tool.
- The **VitaDeck Runtime API** may be implemented as a thin local TypeScript module for the MVP.
- The **VitaDeck SDK** is imported by Deck App projects as `@vitadeck/sdk`, resolved from the npm registry in standalone projects and via the **Deck App Workspace** during VitaDeck development.
- The published **VitaDeck SDK** is consumed like a typical npm library: compiled JavaScript for runtime imports and declaration files for TypeScript.
- The **VitaDeck SDK** exposes the **VitaDeck Runtime API** and produces **Deck App Package Directories**.
- The **VitaDeck SDK** emits a **Runtime Upload Archive** alongside each built **Deck App Package Directory** by default; a build may omit the archive when the author only needs the directory.
- The **VitaDeck SDK** writes **`Deck App Package Version`** into the **Deck App Package Manifest** using the Deck App project's `package.json` `version` by default.
- An optional `version` in `vitadeck.config.json` overrides `package.json` for **`Deck App Package Version`**.
- The **VitaDeck Runtime Bundle** is preferably named `runtime.js` to distinguish it from **Deck App Package Entries**.
- The **VitaDeck Runtime API** provides the **Runtime Theme**.
- **Deck Apps** consume the **Runtime Theme**; VitaDeck owns the theme provider.
- VitaDeck-owned native bridge entry points used to drive rendering and input are not part of the **VitaDeck Runtime API**; **Deck App** source is not written against them.
- The **Screen** component is the root of a **Deck App** UI.
- A **Deck App Component** explicitly renders **Screen**.
- **Button** is the only public interactable UI component in the MVP.
- **Button** exposes press-oriented callbacks: `onPress`, `onPressStart`, and `onPressEnd`.
- **Runtime Upload** and **Host Control** share one **LAN HTTP Listener** on one port; route families differ, not separate always-on servers.
- The **LAN HTTP Listener** binds only when **Runtime Upload Enablement** and/or **Host Control Enablement** is on in the **VitaDeck Shell**; turning off the last enabled feature stops the listener.
- The **LAN HTTP Listener** is not a background daemon while the user only plays a **Deck App** with neither feature enabled.
- **Runtime Upload Enablement** and **Host Control Enablement** are independent shell toggles; either can be on without the other.
- The **LAN HTTP URL** is the single address shown for pairing and upload while the listener runs.
- **Runtime Upload** uses **Runtime Upload Listener** routes on the **LAN HTTP Listener** when **Runtime Upload Enablement** is on.
- The **LAN HTTP Listener** listens on all interfaces with default port **8787** and falls forward to following ports until one binds or **10** attempts fail; the **LAN HTTP URL** reflects the bound port when binding succeeds.
- If all **10** listen attempts fail, the **Shell Upload Screen** shows a bind-failure state (no **Runtime Upload URL**, no **Runtime Upload Web UI**); **Shell Upload Cancel** returns to **Shell Home Screen** without an in-screen retry control.
- **Runtime Upload** primary author interaction is the **Runtime Upload Web UI** served locally by the **Runtime Upload Listener**.
- The **Runtime Upload HTTP Contract** limits documented listener behavior to **`GET /`** (and required same-origin Web UI assets) and **`POST /upload`**; other paths **404**, wrong method on **`/upload`** **405**; no **GET /health**-style probe in the initial **Runtime Upload** iteration.
- Initial **Runtime Upload** trusts the local network; **Upload Pairing** is optional future hardening.
- **Runtime Upload** accepts exactly one **Runtime Upload Archive** per request via **Runtime Upload POST**, using `multipart/form-data` for browsers (file field **`archive`**) and `application/zip` for simple HTTP clients.
- **Runtime Upload POST** returns JSON only: success includes **`packageName`** and **`version`**; failures include **`code`** and **`message`**, with HTTP status reflecting the error class.
- **Runtime Upload Single-Flight** applies to **Runtime Upload POST**: concurrent ingests get **409** and no queue.
- **Runtime Upload** enforces **Runtime Upload Limits** and rejects over-limit archives without installing anything.
- **Runtime Upload** validates packages in **Runtime Upload Staging** and only publishes successful installs into the **Installed Deck App Library**.
- When a simple atomic rename/swap is practical, **Runtime Upload** publishes by swapping staged content into place; otherwise it keeps staging isolated until a single publish step completes.
- **Runtime Upload** replaces an existing **Installed Deck App** when the **Deck App Package Name** matches the uploaded `.vdapp` directory name.
- **Runtime Upload Replacement** is not **Installed Deck App Removal**; it overwrites package contents in place.
- When **Runtime Upload Replacement** targets the **Active Deck App**, VitaDeck performs a **Deck App Runtime Restart** immediately after a successful publish so the running session loads the new package bits.
- Changing the **Active Deck App** at runtime triggers a **Deck App Runtime Restart**.
- After a successful **Runtime Upload** publish, VitaDeck returns to the **Shell Home Screen** with the **VitaDeck Shell** still visible, updates the **Installed Deck App Library** list, and keeps the **Runtime Upload Listener** running so another upload can proceed without reopening the listener; hiding the shell to the **Deck App** or **Shell Upload Cancel** from the **Shell Upload Screen** stops the listener.
- After a failed **Runtime Upload**, VitaDeck keeps the **Shell Upload Screen** open so uploads can be retried without reopening it.
- Runtime Upload installs into the **Installed Deck App Library**; activation is a separate choice.
- **Runtime Upload** successful publishes are **durable**: they survive VitaDeck restarts until **Installed Deck App Removal**; only **Runtime Upload Staging** is temporary.
- The **VitaDeck Shell** is opened through **System Input** rather than by a **Deck App**.
- The **VitaDeck Shell** uses **Shell Face Input** conventions separate from **Deck App** **Press** semantics.
- On macOS, **Host Shell Confirm Mapping** and **Host Shell Back Mapping** apply only while the **VitaDeck Shell** has focus.
- When a gamepad is available on macOS, **Shell Face Input** uses the gamepad face buttons as the primary mapping.
- **Start Input** toggles between the **Deck App** and **VitaDeck Shell** when not on the **Shell Upload Screen**.
- On the **Shell Upload Screen**, **Start Input** does not dismiss the screen; **Shell Upload Cancel** returns to the **Shell Home Screen** and stops the **Runtime Upload Listener**. On the **Shell Home Screen**, **Start Input** or back to hide the shell stops a **Runtime Upload Listener** that is still running after a successful publish.
- **Shell Upload Cancel** is on-screen only on macOS at first; optional host keyboard shortcuts are future work.
- The Vita `Start` button is a **System Input** reserved for VitaDeck; on macOS it uses the **Host Start Mapping** (F1).
- The **Shell Home Screen** lists **Deck App Package Version** from each installed **Deck App Package Manifest**, sorted by manifest **`name`** (case-insensitive), then **Deck App Package Name** when **`name`** ties.
- **Runtime Upload** is offered from the **Shell Home Screen** as a primary action via **Shell Upload Entry**, not as a secondary or **Deck App**-hosted flow.
- **Installed Deck App Removal** cannot target the **Active Deck App**.

## Example dialogue

> **Dev:** "Can this React component be the whole thing VitaDeck runs?"
> **Domain expert:** "For MVP language, call the runnable thing a **Deck App**; a React component may be its entry point, but the Deck App is the user-facing unit."
>
> **Dev:** "Should MVP support uploading Deck Apps while VitaDeck is already running?"
> **Domain expert:** "No — MVP can bundle a **Deck App Source Entry** at build time, but the model should leave room for **Runtime Upload** later."
>
> **Dev:** "Does VitaDeck keep its own navigation UI around the Deck App?"
> **Domain expert:** "No — for MVP, the running **Deck App** owns the full **Render Surface**."
>
> **Dev:** "Should Deck App authors use raw `<vita-button>` tags?"
> **Domain expert:** "No — expose a **VitaDeck Runtime API** so the author contract can stay stable while internals evolve."
>
> **Dev:** "Should Deck App projects typecheck against VitaDeck's native rendering calls?"
> **Domain expert:** "No — those belong to the **VitaDeck Runtime Bundle**. Authors use the **VitaDeck Runtime API**; ordinary JS conveniences the runtime provides (such as timers, a wall clock, and logging) can be typed as a narrow host layer, but native bridge entry points stay internal until VitaDeck deliberately exposes them as API."
>
> **Dev:** "Should the npm package be called `@vitadeck/runtime`?"
> **Domain expert:** "No — use the **VitaDeck SDK** package at `@vitadeck/sdk`; it includes the Runtime API plus build tooling."
>
> **Dev:** "Does the VitaDeck-owned JS bundle still need to be called `main.js`?"
> **Domain expert:** "No — prefer `runtime.js` for the **VitaDeck Runtime Bundle**, though the name is a clarity choice rather than a core domain rule."
>
> **Dev:** "Should each Deck App Package include its own React copy?"
> **Domain expert:** "No — React is a **Runtime Dependency** provided by VitaDeck, and the **VitaDeck SDK** makes the supported development version explicit."
>
> **Dev:** "How do Deck App authors know which React version to use?"
> **Domain expert:** "The **VitaDeck SDK** declares React as a peer dependency and validates the resolved version during package builds."
>
> **Dev:** "Should every Deck App project hand-write its TypeScript config and folder layout?"
> **Domain expert:** "No — the **VitaDeck SDK** provides a TypeScript config preset and a **Deck App Project Scaffold**."
>
> **Dev:** "Is the scaffold for reorganizing the VitaDeck repo?"
> **Domain expert:** "No — a **Deck App Project Scaffold** creates one standalone Deck App project for authors."
>
> **Dev:** "Should each sample app carry its own theme helper?"
> **Domain expert:** "No — theme is a **Runtime Theme** provided by the **VitaDeck Runtime API**."
>
> **Dev:** "Should Deck Apps mount the theme provider?"
> **Domain expert:** "No — VitaDeck provides it during bootstrap; Deck Apps only consume the **Runtime Theme**."
>
> **Dev:** "What should the root UI component be called?"
> **Domain expert:** "Use **Screen**; it represents the Vita display that the Deck App owns."
>
> **Dev:** "Should VitaDeck wrap Deck Apps in Screen automatically?"
> **Domain expert:** "No — a **Deck App Component** explicitly renders **Screen** so display ownership is visible in author code."
>
> **Dev:** "Can any visual element receive clicks?"
> **Domain expert:** "No — **Button** is the only public interactable component in the MVP."
>
> **Dev:** "Should Deck Apps use mouse event names?"
> **Domain expert:** "No — public input is described as **Press**, using `onPress`, `onPressStart`, and `onPressEnd`."
>
> **Dev:** "Should a Deck App call render itself?"
> **Domain expert:** "No — the **Deck App Source Entry** default-exports a **Deck App Component**, and VitaDeck owns rendering it."
>
> **Dev:** "Should app authors call `vitadeck.registerDeckApp`?"
> **Domain expert:** "No — authors only default-export a **Deck App Component**; the **VitaDeck SDK** generates the **Deck App Package Entry** that registers it."
>
> **Dev:** "Does hiding registration behind a global prevent authors from using it?"
> **Domain expert:** "No — a global is still reachable; the **Package Registration Hook** is an internal convention and compatibility boundary, not access control."
>
> **Dev:** "Does the generated registration model mean authors cannot use TypeScript or ES modules?"
> **Domain expert:** "No — author code remains normal TypeScript with ES modules; only the generated **Deck App Package Entry** is shaped for VitaDeck runtime loading."
>
> **Dev:** "Does the Vita runtime need to read the manifest in the MVP?"
> **Domain expert:** "No — the **Deck App Manifest** is build-time metadata for now, but it should be suitable to travel with a future Runtime Upload package."
>
> **Dev:** "Should a `.vdapp` directory include metadata VitaDeck can read?"
> **Domain expert:** "Yes — every **Deck App Package Directory** includes a versioned **Deck App Package Manifest** so VitaDeck can validate the package before loading code."
>
> **Dev:** "Should the built artifact include all of VitaDeck's runtime code?"
> **Domain expert:** "No — call the artifact a **Deck App Package**; it contains the Deck App's compiled code and metadata, while VitaDeck keeps owning runtime bootstrap."
>
> **Dev:** "Should the first package format be a zip file?"
> **Domain expert:** "No — start with a **Deck App Package Directory** so VitaDeck reads plain files; **Runtime Upload** later uses a **Runtime Upload Archive** zip for transport."
>
> **Dev:** "Should `outDir` include the `.vdapp` folder name?"
> **Domain expert:** "No — `outDir` names the parent output location; the **VitaDeck SDK** generates the **Deck App Package Name** with a `.vdapp` suffix."
>
> **Dev:** "How do I avoid passing the Deck App path every time I build?"
> **Domain expert:** "Use project configuration to choose the built **Deck App Package Directory** once, then normal VitaDeck builds copy it."
>
> **Dev:** "Should example Deck Apps stay under `js/deck-apps/`?"
> **Domain expert:** "No — make `js/` a pnpm **Deck App Workspace** with standalone examples under `js/examples/` and the local **VitaDeck SDK** under `js/packages/sdk/`."
>
> **Dev:** "When a user makes another Deck App active at runtime, should VitaDeck hot-swap it inside the existing JavaScript context?"
> **Domain expert:** "No — changing the **Active Deck App** restarts the Deck App runtime with a fresh JavaScript context."
>
> **Dev:** "Should uploading a Deck App automatically make it active?"
> **Domain expert:** "No — Runtime Upload installs into the **Installed Deck App Library**; activation is a separate choice."
>
> **Dev:** "Should the management UI be rendered inside every Deck App?"
> **Domain expert:** "No — use a separate **VitaDeck Shell** opened by **System Input**, with the Vita `Start` button reserved for VitaDeck."
>
> **Dev:** "Should the upload HTTP listener run the whole time VitaDeck is open?"
> **Domain expert:** "No — use a **Runtime Upload Listener** tied to the **VitaDeck Shell**: start it when **Runtime Upload** opens, stop it when the user hides the shell to the **Deck App** or cancels from the **Shell Upload Screen**; after a successful publish it can stay up on **Shell Home Screen** for another upload without rebinding."
>
> **Dev:** "Should Runtime Upload stream individual package files, or one archive?"
> **Domain expert:** "One **Runtime Upload Archive** zip so installs are atomic and easy to validate."
>
> **Dev:** "Should the zip place `manifest.json` at the archive root, or inside a `.vdapp` folder?"
> **Domain expert:** "Inside exactly one top-level `.vdapp` directory, matching the built **Deck App Package Directory** layout."
>
> **Dev:** "If Runtime Upload targets an existing **Deck App Package Name**, should VitaDeck keep both copies?"
> **Domain expert:** "No — replace the installed package, and show **Deck App Package Version** from the **Deck App Package Manifest** in the **VitaDeck Shell**."
>
> **Dev:** "What field name should carry **Deck App Package Version**, and should it be semver?"
> **Domain expert:** "Use `version` in `manifest.json` as a semver string, distinct from `schemaVersion`."
>
> **Dev:** "Where do authors set **Deck App Package Version** during development?"
> **Domain expert:** "Default to `package.json` `version`, with an optional override in `vitadeck.config.json` when present."
>
> **Dev:** "Should **Runtime Upload** require an ephemeral upload token?"
> **Domain expert:** "Not initially — trust the LAN while the **Shell Upload Screen** is open; add optional **Upload Pairing** later if needed."
>
> **Dev:** "Should **Runtime Upload** allow huge zips?"
> **Domain expert:** "No — enforce **Runtime Upload Limits** on uploaded size, unpacked size, and entry count, and reject failures without partial installs."
>
> **Dev:** "Should installs be atomic even if it complicates the implementation?"
> **Domain expert:** "Prefer atomic swap when it stays a small filesystem operation; otherwise still stage and validate, but avoid showing broken half-installed packages in the library."
>
> **Dev:** "Can users delete the **Active Deck App** from the library?"
> **Domain expert:** "No — **Installed Deck App Removal** must not target the **Active Deck App**."
>
> **Dev:** "Can **Runtime Upload** replace the **Active Deck App** anyway?"
> **Domain expert:** "Yes — **Runtime Upload Replacement** overwrites in place, then restarts the Deck App runtime so the active session matches the new bits."
>
> **Dev:** "Should that restart wait for an extra confirmation step?"
> **Domain expert:** "No — do a **Deck App Runtime Restart** immediately after a successful publish to avoid mismatched on-disk bits versus loaded code."
>
> **Dev:** "After a successful upload, should the **Shell Upload Screen** stay open?"
> **Domain expert:** "No — leave the **VitaDeck Shell** on **Shell Home Screen** so the updated library is visible and the listener can keep serving another upload; only hide to the **Deck App** when the author is done managing packages."
>
> **Dev:** "If an upload fails validation, should the Upload screen close?"
> **Domain expert:** "No — keep the **Shell Upload Screen** open so the **Runtime Upload Listener** stays available for a quick retry."
>
> **Dev:** "What macOS key should represent Vita `Start`?"
> **Domain expert:** "Use **Host Start Mapping** on F1."
>
> **Dev:** "Should `Start` dismiss the **Shell Upload Screen** instantly?"
> **Domain expert:** "No — keep **Start Input** from closing uploads; use explicit **Shell Upload** controls so uploads are not ended accidentally."
>
> **Dev:** "What should **Shell Upload Cancel** do?"
> **Domain expert:** "Stop the **Runtime Upload Listener**, return to the **Shell Home Screen**, and keep the **VitaDeck Shell** open."
>
> **Dev:** "Should **Shell Upload Cancel** have a macOS keyboard shortcut immediately?"
> **Domain expert:** "No — keep it on-screen only at first to avoid accidental listener shutdown."
>
> **Dev:** "Should **VitaDeck Shell** controls use the same input words as **Deck App** buttons?"
> **Domain expert:** "No — **Shell Face Input** uses Cross/Circle confirm/back conventions; **Deck Apps** keep **Press** language."
>
> **Dev:** "How should **Shell Face Input** work on macOS with only a keyboard?"
> **Domain expert:** "Prefer a gamepad when present; otherwise use **Host Shell Confirm Mapping** on Enter and **Host Shell Back Mapping** on Backspace while the Shell is focused."
>
> **Dev:** "Should **Runtime Upload** be a raw HTTP upload only?"
> **Domain expert:** "No — serve a **Runtime Upload Web UI** at the **Runtime Upload URL** for drag/drop uploads and clear browser-side success/failure, backed by the same **Runtime Upload Listener**."
>
> **Dev:** "Should browser uploads use multipart zip posts?"
> **Domain expert:** "Yes — use `multipart/form-data` as the primary **Runtime Upload POST** shape with a single file part named **`archive`**, and keep raw `application/zip` as a compatible option for scripts."
>
> **Dev:** "Should **Runtime Upload POST** return HTML for browsers and plain text for scripts?"
> **Domain expert:** "No — JSON only for every client: **`ok`**, **`packageName`** and **`version`** on success, **`code`** and **`message`** on failure, with HTTP status for the error class."
>
> **Dev:** "How should the **Runtime Upload Listener** choose host and port?"
> **Domain expert:** "Bind on all interfaces for LAN access, start at a default port (**8787**) and try the next ports if busy (up to **10** tries), and show the real **Runtime Upload URL** on the **Shell Upload Screen**."
>
> **Dev:** "What if two **Runtime Upload POST** requests overlap?"
> **Domain expert:** "Use **Runtime Upload Single-Flight**: reject the second with **409** and JSON failure; no queue, so VitaDeck stays simple."
>
> **Dev:** "If every port in the fallback range is busy, what should the **Shell Upload Screen** offer?"
> **Domain expert:** "Show that the upload server could not start, omit the **Runtime Upload URL**, and rely on **Shell Upload Cancel** to go home — no mandatory retry button."
>
> **Dev:** "Should the **Runtime Upload Listener** expose extra HTTP APIs for scripts?"
> **Domain expert:** "No — stick to the **Runtime Upload HTTP Contract**: **`GET /`** plus assets for the **Runtime Upload Web UI** and **`POST /upload`**; **404** elsewhere, **405** on **`/upload`** if not **POST**; skip a separate health endpoint for now."
>
> **Dev:** "Should **Runtime Upload** be a buried Shell action?"
> **Domain expert:** "No — **Shell Upload Entry** on the **Shell Home Screen** is a primary control using normal **Shell Face Input**, same family as picking a library item."
>
> **Dev:** "Are **Runtime Upload** installs session-only?"
> **Domain expert:** "No — after publish they are normal **Installed Deck App Library** entries on disk until removed, same durability expectation as build-time shipped packages."
>
> **Dev:** "How should the **Shell Home Screen** order installed apps?"
> **Domain expert:** "Alphabetical by **`name`** in **Deck App Package Manifest**, case-insensitive, with **Deck App Package Name** as a stable tie-break."
>
> **Dev:** "If Host Control shares the upload HTTP port, does the server run all the time?"
> **Domain expert:** "No — one **LAN HTTP Listener**, but it only binds when the user turns on **Runtime Upload Enablement** and/or **Host Control Enablement** in the Shell. Playing a **Deck App** with both off means no LAN server."
>
> **Dev:** "Can I upload a Deck App without enabling Host Control?"
> **Domain expert:** "Yes — enablements are independent; only the enabled route families are meaningful while the listener is up."

## Flagged ambiguities

- "React component" can mean either a source-level entry component or the whole runnable **Deck App**; resolved: **Deck App** is the runnable unit.
- "upload" can mean build-time packaging or **Runtime Upload**; resolved: MVP uses build-time bundling, while **Runtime Upload** is a future HTTP-based flow.
- "HTTP upload" could mean browser multipart or raw zip bytes; resolved: **Runtime Upload POST** supports `multipart/form-data` for browsers (field **`archive`**) and `application/zip` for automation.
- "upload response" could mean HTML, plain text, or structured data; resolved: **Runtime Upload POST** always responds with JSON and uses HTTP status for error class.
- "concurrent uploads" could mean parallel ingests or queued retries; resolved: **Runtime Upload Single-Flight** — one ingest at a time, **409** for overlap, no queue.
- "UI elements" can mean public author-facing components or internal host elements; resolved: **Deck Apps** use the **VitaDeck Runtime API**, while host elements remain internal.
- "`runtime` package" is too narrow for the package that also builds artifacts; resolved: use **VitaDeck SDK** for `@vitadeck/sdk`.
- "React dependency" could mean bundled app code or a VitaDeck-provided runtime dependency; resolved: React is a **Runtime Dependency** and Deck App projects develop against the SDK-compatible version.
- "theme" can mean sample app styling or runtime styling; resolved: the shared theme is a **Runtime Theme** owned by VitaDeck.
- "clickable rect" suggests visual primitives are interactive; resolved: public interaction goes through **Button**.
- "click" and "mouse" terms are implementation leakage; resolved: public input language is **Press**.
- "confirm" could mean **Deck App** **Press** or **VitaDeck Shell** confirmation; resolved: **Shell Face Input** uses Cross/Circle conventions in the Shell, while **Deck Apps** use **Press**.
- "Enter" could mean **Deck App** activation or **Shell Face Input** confirm; resolved: **Host Shell Confirm Mapping** applies only while the **VitaDeck Shell** has focus.
- "Surface" and "viewport" were considered for the root display component; resolved: the public component is **Screen**.
- "render function" suggests author-controlled bootstrapping; resolved: authors export a **Deck App Component** and VitaDeck bootstraps it.
- "entry" can mean author source or generated package code; resolved: **Deck App Source Entry** is authored, **Deck App Package Entry** is generated.
- "generated registration" could imply restricted author syntax; resolved: author code supports TypeScript and ES modules before SDK compilation.
- "manifest" could mean runtime configuration or build metadata; resolved: **Deck App Manifest** is build-time metadata in the MVP.
- "artifact" could mean either a complete VitaDeck runtime bundle or a portable **Deck App Package**; resolved: the package excludes the VitaDeck runtime.
- "package" could mean an archive or a directory; resolved: the on-disk artifact is a **Deck App Package Directory**, while **Runtime Upload** uses a **Runtime Upload Archive** for transport.
- "zip layout" could mean flat files at the archive root or a nested `.vdapp` folder; resolved: **Runtime Upload Archive** requires exactly one top-level `.vdapp` directory.
- "`schemaVersion`" can be mistaken for the app release version; resolved: use **Deck App Package Schema Version** vs **Deck App Package Version**.
- "`version`" could mean npm package versioning or **Deck App Package Version**; resolved: **Deck App Package Version** is the `version` field in `manifest.json`, semver-shaped for VitaDeck UI, sourced from `package.json` by default and overridden by optional `vitadeck.config.json` `version`.
- "`outDir` could mean the final package directory or its parent; resolved: `outDir` is the parent and the **VitaDeck SDK** generates the `.vdapp` directory name.
- "**Deck App Workspace** no longer means only in-repository app source; resolved: it is the JavaScript pnpm workspace for SDK, runtime, and examples.
- "selected app" is the **Active Deck App**; resolved: project configuration can select it for build-time packaging, and Runtime Upload can select it on-device.
- "upload" does not imply activation; resolved: Runtime Upload installs into the **Installed Deck App Library**, and activation is explicit.
- "upload install" could mean RAM-only or until reboot; resolved: published **Runtime Upload** packages persist like other **Installed Deck App Library** entries; staging alone is ephemeral.
- "Vita UI" could mean a Deck App screen or VitaDeck management UI; resolved: management belongs to the **VitaDeck Shell**.
- "`Start` input" is not Deck App input; resolved: it is **Start Input** (**System Input**) reserved for VitaDeck, mapped on macOS via **Host Start Mapping** (F1), toggling between the **Deck App** and **VitaDeck Shell** except on the **Shell Upload Screen**.
- "cancel upload" could mean leaving the Shell entirely; resolved: **Shell Upload Cancel** returns to **Shell Home Screen** while keeping the Shell open.
- "cancel control" could imply a host keyboard binding; resolved: **Shell Upload Cancel** is on-screen only on macOS at first.
- "HTTP server on Vita" could imply an always-on network service; resolved: use a **LAN HTTP Listener** started only when **Runtime Upload Enablement** and/or **Host Control Enablement** is on in the **VitaDeck Shell**, stopped when all are off (not a background daemon during normal **Deck App** play).
- "shared listener" could imply always-on; resolved: one **LAN HTTP Listener** multiplexes routes, but enablement stays explicit like today's upload toggle.
- "Runtime Upload Listener" vs **LAN HTTP Listener** could imply two servers; resolved: **Runtime Upload Listener** is the upload route family on the shared **LAN HTTP Listener**.
- "upload server API" could sprawl into many routes; resolved: **Runtime Upload HTTP Contract** — **`GET /`** + Web UI assets and **`POST /upload`** only as the documented surface; **404**/**405** as above; no health endpoint initially.
- "upload URL" could hide port collisions or loopback-only binds; resolved: **Runtime Upload URL** uses LAN IP and the port actually bound; default **8787** with consecutive fallback up to **10** attempts; all interfaces.
- "ports exhausted" could leave the user stuck or imply auto-retry; resolved: **Shell Upload Screen** bind-failure state, no URL, **Shell Upload Cancel** only (no required in-screen retry).
- "Upload UI" could mean a **Deck App** flow, a **VitaDeck Shell** flow, or a browser page; resolved: **Runtime Upload** reaches the **Shell Upload Screen** from the **Shell Home Screen** via primary **Shell Upload Entry**, then uses the browser **Runtime Upload Web UI** at the **Runtime Upload URL**.
- "upload finished" could mean success or failure; resolved: success returns to **Shell Home Screen** (listener may keep running); failure keeps the **Shell Upload Screen** open for retry.
- "secure upload" could imply authenticated clients; resolved: initial **Runtime Upload** is LAN-trusted, with optional future **Upload Pairing**.
- "trusted LAN" could imply unlimited resource use; resolved: still enforce **Runtime Upload Limits** and fail closed without partial installs.
- "atomic install" could imply heavy transactional machinery; resolved: use **Runtime Upload Staging** plus a swap/publish step only when it stays simple; never surface invalid packages as installed.
- "delete app" could include deleting the running **Deck App**; resolved: disallow **Installed Deck App Removal** for the **Active Deck App**.
- "replace" could sound like **Installed Deck App Removal**; resolved: **Runtime Upload Replacement** overwrites package contents and may restart the active session, but does not remove the active selection.
- "restart" could mean hot reload versus a full context reset; resolved: use **Deck App Runtime Restart** for a fresh JavaScript context.
- "globals available in Deck App code" can mean ordinary JS host conveniences or VitaDeck native bridge entry points; resolved: the author-facing surface is the **VitaDeck Runtime API** plus a small typed host subset for normal patterns, not native bridge calls.
- "app list order" could mean install order, recent use, or arbitrary; resolved: **Shell Home Screen** sorts by manifest **`name`** (case-insensitive), tie-break **Deck App Package Name**.
