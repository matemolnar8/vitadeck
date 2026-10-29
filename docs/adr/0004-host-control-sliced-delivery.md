# Host Control: the companion listens, plugins define the commands

**Host Control** lets a **Deck App** ask a nearby computer to run a typed command. Draft PR #16 designed and built it as one 50-file change: SDK contract, companion CLI, a `sceHttp` client on the Vita, vendored picohttpparser, a rewritten upload server, and an always-on LAN listener that changed the **Runtime Upload** lifecycle. It was closed because it could not be reviewed or rebased. This ADR keeps the parts of that design that held up and drops the rest.

## Delivery

Host Control was planned as three slices (#109 wire contract and SDK client, #110 Host Companion CLI, #111 native link and Shell screen). They landed together in one PR on request, as one commit per slice, and the slice boundaries still hold: the SDK client needs neither a Vita nor a companion in its tests, the companion tests against a TypeScript stand-in for the Vita link endpoint, and only the native slice needs the `full` agent-check tier. All three replay the same contract fixtures in `tests/fixtures/host-control/`.

## Topology

Link once, then the Vita calls the companion. This is option C in #16's topology comparison.

The user reads an address on the **Shell Host Control Screen** and gives it to the **Host Companion**. The companion sends one link request with its protocol version, a stable id, its name, and the callback URL where it accepts commands. VitaDeck stores that URL and sends each later command to it as an HTTP POST. The companion is the long-lived listener. The Vita listens only while the Shell Host Control Screen is open, and stops when a link succeeds or the user leaves. That matches the **Runtime Upload Listener** lifecycle, and the link endpoint stays separate from that listener rather than becoming #16's always-on server shared with upload.

This is the only option that meets the pairing goal ("type the Vita's address on the computer") without running a session server on the Vita. Steady-state commands need no new native code: Deck Apps already have `fetch()` on libcurl, with completions on a worker thread, on both targets. That makes #16's 650-line `sceHttp` client unnecessary. The Vita is exposed on the LAN only briefly, and only when the user asks for it. A new TCP connection per command costs a few milliseconds on a home LAN, which is fine for button presses and for polling a gauge once a second.

**Several computers.** VitaDeck keeps up to 4 links, newest first, keyed by the companion's id, so one Deck App can drive several machines (`createHostClient(plugin, { hostId })`) and a relink replaces rather than duplicates. One companion can likewise be linked by several Vitas.

Accepted costs:
- The companion needs an inbound firewall rule.
- A stale callback URL (a DHCP change, or the companion restarting on another port) breaks commands until the user links again.
- The computer cannot push events to the Vita; a Deck App polls. Alerts (a build finished, a session needs attention) arrive within one poll interval.

**Alternative (option B):** the companion dials in and holds a persistent WebSocket or long-poll session with the Vita. That gives lower per-command latency and lets the computer push to the Vita. The cost is a WebSocket or long-poll server, a command queue, and reconnect logic in Vita C, which is the riskiest native work in this initiative. Revisit it only if polling proves too slow or too costly on the Vita.

## Commands come in plugins

A fixed catalogue in the companion cannot cover what Deck Apps will want (keyboard shortcuts, media keys, a Stream Deck-style grid, sensors and fan control, a T3 Code dashboard), and a generic "run this shell command" endpoint loses types and invites misuse. So commands are grouped into **Host Control Plugins**. A plugin is a typed registry (`defineHostPlugin(id, { name: command<Payload, Result>(guard) })`) that a Deck App imports and bundles, plus an implementation (`implementHostPlugin(registry, implementations)`) that the companion runs. Every request names its plugin and command; the companion validates the payload with the registry's guard, and TypeScript fails a missing or mistyped implementation. `capabilities` lists every loaded plugin, so a Deck App can tell a computer without a plugin (`unknown_plugin`) from an offline one.

There is no schema library, since one would ship inside every Deck App bundle. The protocol version covers only the envelope; a plugin changes its own command names when it breaks them.

**Built-in plugins first.** The companion ships the plugins most Deck Apps need, and the SDK exports their registries, so a Deck App that uses only those needs nothing installed beyond the companion. Today that is `core` (`capabilities`, `echo`) and the read-only `systemStats`. Keys and shortcuts, media keys and opening apps or URLs are the next built-ins, and each waits for credentials (see Deferred). Plugins tied to one program, like a T3 Code dashboard, or to one machine, like fan control, stay custom.

**A custom plugin lives in a project, with or without a Deck App, and ships as a package, like the Deck App.** The author adds `hostPlugin` (`entry`, optional `files`) to `vitadeck.config.json`: next to a Deck App's `entry` (`create --host-plugin`), or alone in a plugin project (`create --host-plugin-only`) whose `package.json` exports the command registry, so any number of Deck Apps depend on it the way they depend on any package and its users install it once. `vitadeck build` writes the **Host Plugin Package**, a `<slug>.vdplugin` folder with `manifest.json`, the implementation bundled with its npm dependencies and the SDK code it uses into one ES module (`companion.mjs`), and the `files` folder, then zips it to `<slug>.vdplugin.zip` next to the `.vdapp.zip`. It is a folder and a zip rather than one JavaScript file because plugins will carry more than code (scripts, data, native addons a bundle cannot inline); the entry finds them through `import.meta.url`, and the manifest's `schemaVersion` leaves room to grow. The zip keeps no file modes, so a plugin runs a shipped script through its interpreter rather than executing it directly. The user runs `vitadeck-host-companion install <archive>`, which unpacks it into `~/.vitadeck/plugins/<id>.vdplugin`, and a running companion reloads that folder and every `--plugin` path when they change, so `vitadeck dev` plus `start --plugin ./dist` is the author's edit loop. The companion imports the entry by file URL, with a query of its content hash so a rebuild loads fresh instead of from the module cache. It checks the default export's shape (`id`, a `Map` of commands) and a `HostCommandError`'s name and code rather than classes, since each package bundles its own copy of the SDK. The manifest and that shape are the contract between a package built with one SDK version and a companion of another.

Considered for custom plugins: npm packages the companion resolves by name (rejected: an author would publish a package per Deck App, and a global companion cannot resolve packages installed elsewhere); one bundled JavaScript file (rejected: it cannot carry anything but code); and serving the implementation from the Vita at link time (rejected: the Vita would push code that the computer runs).

**Two CLIs, one per machine role.** `@vitadeck/sdk`'s `vitadeck` builds Deck Apps and Host Plugin Packages; `@vitadeck/host-companion`'s `vitadeck-host-companion` runs on any computer a Vita controls. A single `vitadeck host ...` would make every user install the Deck App build toolchain (Babel, rolldown) to run a server. A second bin in the SDK would also break `npx @vitadeck/sdk`, which works only while the package has exactly one bin. The companion bundles the SDK code it needs and has no dependencies.

## HTTP parsing

Extend the upload server's parsing; do not vendor picohttpparser.

The Vita parses exactly one new kind of request: a small JSON link POST with `Content-Length`, answered with `Connection: close`. The request reading, response writing, and JSON error helpers moved from `src/upload/http_server.c` into `src/net/http.c`, which both listeners call; header names match case-insensitively, since Node's `fetch` sends `content-length`. picohttpparser remains the fallback if the Vita ever needs keep-alive, pipelining, or chunked bodies, which would happen if option B is chosen.

## Deferred

- **Credentials.** v1 trusts the LAN, like the initial **Runtime Upload**. A linked companion answers only its Vitas, loopback and its own addresses, but IPs can be spoofed. That is harmless for reading stats, but not for typing keys or launching apps, so a credential decision must come before the first plugin that acts on the computer.
- **Publishing.** `@vitadeck/host-companion` is built as a dependency-free package but stays private until its first release; until then `pnpm --dir js host-companion` runs it from a checkout.
- **Internet or account pairing.** Host Control is LAN only.

**Host Control Link** is not **Upload Pairing**. A link exchanges an address and authorizes nothing. **Upload Pairing** is a future, ephemeral credential scoped to the **Runtime Upload Listener**. When Host Control needs a credential, the likely shape is the same idea: a short-lived code shown on the Vita and entered on the companion. At that point, either widen **Upload Pairing** to cover both listeners or add a sibling term. Decide then; do not overload the term now.

**Considered options:** rebase #16 (rejected: four months stale, and it couldn't be reviewed as one change). Topology A, where the user types the computer's address on the Vita (rejected: wrong pairing direction, and it needs a Shell text editor). Topology B (see the alternative above). One always-on LAN listener for upload and Host Control (rejected: it changes the Runtime Upload lifecycle for no Host Control need). A second HTTP client on `sceHttp` (rejected: `fetch()` already covers it). A fixed command catalogue or a shell-command endpoint (rejected: see plugins above).

**Consequences:** the Runtime Upload lifecycle and its glossary entries stay as they are. If option B is chosen instead, rewrite this ADR and the HTTP parsing recommendation together.
