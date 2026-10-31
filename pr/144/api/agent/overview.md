Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify ae5083caa9370b5f5985de1de0aa147a1afecc2e` in this repository.
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

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               ae5083caa9370b5f5985de1de0aa147a1afecc2e

Named refs at pin: a447e3b9 ... ae5083ca

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f ae5083caa9370b5f5985de1de0aa147a1afecc2e

Commits:
- ae5083ca fix(host-companion): relink attempts run one at a time and re-read the config
- 2a18bdfe feat(host-control): the companion relinks its Vitas on every start
- fbb6af02 refactor(shell): read the row count once when Home opens
- df415e34 fix(shell): an install keeps Home focus on the entry row it was on
- 70e9ba2b fix(shell): an empty Shell focuses Host Control so Cross keeps listening
- f9245aa2 feat(shell): one LAN Listener for upload and link, on from startup
- 82350502 refactor(host-companion): put the lifecycle comment on the call it explains
- 18acd4f7 fix(sdk): a watch callback that stops watching ends it at once
- 7f2c1726 fix(host-companion): start plugin lifecycles after binding the port
- 20461a88 feat(host-control): computers push events to Deck Apps by long-polling
- 0ec52014 docs(skills): add vitadeck-host-control and cover Host Control in the others
- 51440f28 test(sdk): pin the standalone plugin's peer SDK and exclusive create flags
- 5820312a docs(sdk): say which SDK copy a linked standalone plugin uses
- 607fcd29 fix(sdk): standalone plugins take the SDK as a peer dependency
- c41d480c feat(sdk): Host Plugins can be authored without a Deck App
- a90a3cf6 fix(sdk): build the plugin package beside the old one and swap it in
- e069a2f5 fix(host-control): a broken plugin build keeps the last good package
- a601a6ca feat(host-control): a Deck App's plugin ships as a .vdplugin package and zip
- 96448b7b fix(host-companion): --plugin takes a directory, so the scaffold survives a renamed app
- c0498393 fix(host-companion): allow plugin ids as long as a package slug
- 445f5ef3 fix(host-companion): rename the plugin's module copy into place
- 966550e1 fix(host-companion): install renames the plugin into place
- 4139145b fix(host-companion): load plugins from file copies, resolve paths from the cwd
- 148fceef feat(host-control): built-in plugins, .vdplugin files, vitadeck-host-companion
- 04760101 fix(host-companion): reject unlinked peers before reading their body
- eb61ca83 fix(host-control): close a connection accepted while the listener stops
- 77aad963 fix(host-control): show how to link another computer after a link; help before commands
- 3d928f8d fix(host-control): review fixes for polling backoff, result spread, and listener stop
- 959cfda2 feat(examples): Host Control demo Deck App, docs, and agent-check
- 5006b10b feat(host-control): Host Control Link listener, store, getter, and Shell screen
- 9c60f8a2 feat(host-companion): Host Companion CLI and system-stats plugin
- 7130b909 fix(sdk): typecheck and lint the Host Control tests
- 8d3b93f2 feat(sdk): Host Control wire contract, plugins, and client

Sources:
- ticket #109 JSON-over-HTTP contract, error codes, typed registry, `@vitadeck/sdk/host-control` client, shared fixtures.
  https://github.com/matemolnar8/vitadeck/issues/109
- ticket #110 Private companion package with `start` and `link`, config file, peer check, node:test suite.
  https://github.com/matemolnar8/vitadeck/issues/110
- ticket #111 Shared HTTP parsing, LAN address in platform, link listener and store, native getter, Shell entry and screen, C harness.
  https://github.com/matemolnar8/vitadeck/issues/111
- pr PR #144 One PR for all three slices; plugins instead of a flat catalogue; several links; demo Deck App.
  https://github.com/matemolnar8/vitadeck/pull/144
- transcript T3 Code session · Oct 28 Implement all three issues; target use cases shape the design but are not built now; the backend extension model is left to the agent; add a demo Deck App.
- transcript T3 Code session · Oct 29 Máté asked for a better host→Vita channel than polling and approved long-polling in this PR.

The title:

Host Control: let Deck Apps call linked computers through built-in and custom plugins

The why:

Máté asked for the three Host Control slices, shaped so later Deck Apps can send keys, read sensors and set controls, act like a Stream Deck, drive several machines on different OSes, and show a T3 Code dashboard ([session](source:s5)).

The what (very large):

Adds Host Control end to end: the `@vitadeck/sdk/host-control` client and plugin API, the `vitadeck-host-companion` CLI with built-in `core` and `systemStats` plugins that installs custom `.vdplugin` packages, SDK CLI support to build those packages with a Deck App or standalone, a LAN Listener that VitaDeck starts at boot for both Runtime Upload and links, a link store and a Shell Host Control screen, and a `host-control` example Deck App, and events the computer pushes to Deck Apps by long-polling.

Look for:
- [#109](source:s1) asks for flat command names and an internal `defineHostCommands`. Every request instead names a `plugin`, and `defineHostPlugin` and `command` are public, because the [session](source:s5) left the extension model open and [PR #144](source:s4) chose plugins.
- [#109](source:s1) and [#111](source:s3) name the getter `nativeGetHostControlUrl()` and one stored link in `host-control-link.json`. At head `nativeGetHostControlLinks()` returns a JSON array of up to 4 links in `host-control-links.json`, keyed by a new companion `id` in the link payload, so one Deck App can reach several machines ([session](source:s5)).
- [#109](source:s1) asks for a new `test:sdk` script. At head the SDK host-control tests run under the existing `test:cli` glob; no source gives a reason.
- [#110](source:s2) keeps one `vita` in the companion config. At head `vitas` is an array, since several Vitas can link one companion.
- [#111](source:s3) describes a `Forget link` action. At head Cross on a linked computer row forgets that one link at once.
- Credentials stay deferred, as ADR 0004 says, so the built-in plugins only read information; custom plugins can act on the computer and run with the user's permissions.
- Not true at head: `@vitadeck/host-companion` is still `private: true`, so a project from `create --host-plugin` or `--host-plugin-only` cannot `npm install` its companion devDependency until the package is published.
- [#111](source:s3) has the Vita listen for links on 8798 only while the Shell Host Control screen is open. At head `POST /link` is a route on the LAN Listener (8787), which also serves Runtime Upload and starts at boot. After testing, Máté found the setup clunky and asked for upload and link to start together and by default ([session](source:s5)). Anyone on the LAN can now upload and link whenever VitaDeck runs.
- [#110](source:s2) and [#111](source:s3) have the user run `link` again when the callback URL goes stale. At head every companion `start` relinks the Vitas it linked and never re-adds one the user forgot, as Máté asked ([session](source:s5)).

## Review concerns

### 01 Wire contract and SDK client (`sdk-contract`)

`createHostClient` in `client.ts` turns a `defineHostPlugin` registry from `plugin.ts` into typed methods that POST the `protocol.ts` envelope and map every failure to a `HostResult`, and `watchHostEvents` keeps one `core.events` wait open per watcher; `plugin.ts` also gives authors typed `event<D>()` declarations, `implementHostPlugin` with a `start(emit)` lifecycle, `HostCommandError` and the `HostPluginManifest` contract, and `system-stats.ts` holds the built-in `systemStats` registry.

[groups/sdk-contract.md](groups/sdk-contract.md)

### 02 Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

`cli.ts` runs `start`, which serves `POST /command` from `server.ts` over `core`, the built-in `systemStats` and every `.vdplugin` package `plugin.ts` loads from `~/.vitadeck/plugins/` or `--plugin` (reloading them on change through `setPlugins`), `install`/`remove`, which unpack and delete those packages, and `link`, which POSTs the link payload from `link.ts` to the Vita LAN Listener (port 8787 by default) and records the Vita in `config.ts`; every `start` relinks the recorded Vitas with `relink: true`, one at a time, retrying an unanswering one every 30 s and dropping one that answers `unknown_link`; `rolldown.config.mjs` bundles it with the SDK into one dependency-free `vitadeck-host-companion` bin. `notify` sends a `core` notice, `core.events` and `core.notify` live in `core.ts`, and the server starts plugin lifecycles once the port is bound and stops them on reload or close.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

[groups/companion.md](groups/companion.md)

### 03 Events from the computer by long-polling (`events`)

`events.ts` numbers each plugin's last 100 events and holds a `core.events` wait until that plugin's next one or 25 s, and `fetch.c` aborts pending fetches through curl's progress callback when a Deck App stops, which `js_limits_harness` proves with a server that never answers.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)
- 02 Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

[groups/events.md](groups/events.md)

### 04 SDK CLI: build Host Plugin packages, with or without a Deck App (`sdk-cli`)

`hostPlugin: { entry, files? }` in `vitadeck.config.json` makes `build` bundle the entry into a staged `<slug>.vdplugin` folder with `manifest.json` and `files/`, swap it in and zip it, `entry` becomes optional so a plugin-only project builds and `dev`s alone, and `create --host-plugin` / `--host-plugin-only` scaffold the two layouts from the `host-plugin` and `host-plugin-only` templates.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

[groups/sdk-cli.md](groups/sdk-cli.md)

### 05 Shared native helpers moved out of upload and storage (`native-shared`)

Upload's HTTP helpers move to `src/net/http.c`, its LAN address detection to `src/platform/lan_address_*`, jimp and JSON escaping to `src/core/json.c`, and Deck App Storage's replace-file logic to `vd_replace_file` in `src/core/util.c`.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

[groups/native-shared.md](groups/native-shared.md)

### 06 LAN Listener, link store and native getter (`native-link`)

`src/net/lan_listener.c`, the former `src/upload/http_server.c`, adds a `POST /link` route next to `/upload` on 8787+ and `main.c` starts it at boot; `link.c` validates a link and stores up to 4 through `vd_replace_file`, and `host_control_links_relink` updates an existing link in place for a `relink: true` request or answers 404 `unknown_link`, and `nativeGetHostControlLinks` in `jslib/host_control.c` hands the list to the SDK client.

Depends on:
- 05 Shared native helpers moved out of upload and storage (`native-shared`)

[groups/native-link.md](groups/native-link.md)

### 07 Shell listener toggle and Host Control screen (`shell`)

The Upload row becomes `Upload and link`, showing the LAN Listener URL and toggling it, and the Host Control row opens `VD_SHELL_HOST_CONTROL`, which shows the `link` command with the listener address and the linked computers without touching the listener.

Depends on:
- 06 LAN Listener, link store and native getter (`native-link`)

[groups/shell.md](groups/shell.md)

### 08 host-control example Deck App (`demo`)

`App.tsx` polls `linkedHosts()` and each host's built-in `systemStats.read`, imported from the SDK, lists the computers with live CPU, and shows the selected one's capabilities, gauges and an Echo round trip, plus the latest `core` notice it watches with `watchHostEvents`.

Depends on:
- 02 Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

[groups/demo.md](groups/demo.md)

### 09 Glossary, ADR, agent skills and checks (`docs`)

`CONTEXT.md` adds the Host Control Event, Host Control Plugin, Host Plugin Package, LAN Listener and Shell terms, ADR 0004 is rewritten for plugins, long-polled events, built-ins, `.vdplugin` packages, standalone plugins, two CLIs, several links and the LAN Listener on from startup, the new public `vitadeck-host-control` skill and updates to the init, manual-testing, upload and build skills teach agents the feature, `AGENTS.md` lists the new code and skill, and `agent-check.sh fast` runs `test:host-companion`.

[groups/docs.md](groups/docs.md)