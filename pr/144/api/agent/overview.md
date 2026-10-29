Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify 0ec520145e1ff4657a25cec36682c2b86f5e6c15` in this repository.
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

head               0ec520145e1ff4657a25cec36682c2b86f5e6c15

Named refs at pin: a447e3b9 ... 0ec52014

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 0ec520145e1ff4657a25cec36682c2b86f5e6c15

Commits:
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

The title:

Host Control: let Deck Apps call linked computers through built-in and custom plugins

The why:

Máté asked for the three Host Control slices, shaped so later Deck Apps can send keys, read sensors and set controls, act like a Stream Deck, drive several machines on different OSes, and show a T3 Code dashboard ([session](source:s5)).

The what (very large):

Adds Host Control end to end: the `@vitadeck/sdk/host-control` client and plugin API, the `vitadeck-host-companion` CLI with built-in `core` and `systemStats` plugins that installs custom `.vdplugin` packages, SDK CLI support to build those packages with a Deck App or standalone, the native link listener and link store with a Shell Host Control screen, and a `host-control` example Deck App.

Look for:
- [#109](source:s1) asks for flat command names and an internal `defineHostCommands`. Every request instead names a `plugin`, and `defineHostPlugin` and `command` are public, because the [session](source:s5) left the extension model open and [PR #144](source:s4) chose plugins.
- [#109](source:s1) and [#111](source:s3) name the getter `nativeGetHostControlUrl()` and one stored link in `host-control-link.json`. At head `nativeGetHostControlLinks()` returns a JSON array of up to 4 links in `host-control-links.json`, keyed by a new companion `id` in the link payload, so one Deck App can reach several machines ([session](source:s5)).
- [#109](source:s1) asks for a new `test:sdk` script. At head the SDK host-control tests run under the existing `test:cli` glob; no source gives a reason.
- [#110](source:s2) keeps one `vita` in the companion config. At head `vitas` is an array, since several Vitas can link one companion.
- [#111](source:s3) describes a `Forget link` action. At head Cross on a linked computer row forgets that one link at once.
- Credentials stay deferred, as ADR 0004 says, so the built-in plugins only read information; custom plugins can act on the computer and run with the user's permissions.
- Not true at head: `@vitadeck/host-companion` is still `private: true`, so a project from `create --host-plugin` or `--host-plugin-only` cannot `npm install` its companion devDependency until the package is published.

## Review concerns

### 01 Wire contract and SDK client (`sdk-contract`)

`createHostClient` in `client.ts` turns a `defineHostPlugin` registry from `plugin.ts` into typed methods that POST the `protocol.ts` envelope and map every failure to a `HostResult`, while `plugin.ts` also gives plugin authors `implementHostPlugin`, `HostCommandError` and the `HostPluginManifest` contract, and `system-stats.ts` holds the built-in `systemStats` registry.

[groups/sdk-contract.md](groups/sdk-contract.md)

### 02 Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

`cli.ts` runs `start`, which serves `POST /command` from `server.ts` over `core`, the built-in `systemStats` and every `.vdplugin` package `plugin.ts` loads from `~/.vitadeck/plugins/` or `--plugin` (reloading them on change through `setPlugins`), `install`/`remove`, which unpack and delete those packages, and `link`, which POSTs the link payload from `link.ts` and records the Vita in `config.ts`; `rolldown.config.mjs` bundles it with the SDK into one dependency-free `vitadeck-host-companion` bin.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

[groups/companion.md](groups/companion.md)

### 03 SDK CLI: build Host Plugin packages, with or without a Deck App (`sdk-cli`)

`hostPlugin: { entry, files? }` in `vitadeck.config.json` makes `build` bundle the entry into a staged `<slug>.vdplugin` folder with `manifest.json` and `files/`, swap it in and zip it, `entry` becomes optional so a plugin-only project builds and `dev`s alone, and `create --host-plugin` / `--host-plugin-only` scaffold the two layouts from the `host-plugin` and `host-plugin-only` templates.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

[groups/sdk-cli.md](groups/sdk-cli.md)

### 04 Shared native helpers moved out of upload and storage (`native-shared`)

Upload's HTTP helpers move to `src/net/http.c`, its LAN address detection to `src/platform/lan_address_*`, jimp and JSON escaping to `src/core/json.c`, and Deck App Storage's replace-file logic to `vd_replace_file` in `src/core/util.c`.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

[groups/native-shared.md](groups/native-shared.md)

### 05 Link listener, store and native getter (`native-link`)

`listener.c` accepts one `POST /link` on 8798+, `link.c` validates it and stores up to 4 links through `vd_replace_file`, and `nativeGetHostControlLinks` in `jslib/host_control.c` hands the list to the SDK client.

Depends on:
- 04 Shared native helpers moved out of upload and storage (`native-shared`)

[groups/native-link.md](groups/native-link.md)

### 06 Shell Host Control entry and screen (`shell`)

A Host Control row after Upload opens `VD_SHELL_HOST_CONTROL`, which starts the listener, shows the address, status and linked computers, and stops the listener on a link, Circle, Start or shutdown.

Depends on:
- 05 Link listener, store and native getter (`native-link`)

[groups/shell.md](groups/shell.md)

### 07 host-control example Deck App (`demo`)

`App.tsx` polls `linkedHosts()` and each host's built-in `systemStats.read`, imported from the SDK, lists the computers with live CPU, and shows the selected one's capabilities, gauges and an Echo round trip.

Depends on:
- 02 Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

[groups/demo.md](groups/demo.md)

### 08 Glossary, ADR, agent skills and checks (`docs`)

`CONTEXT.md` adds the Host Control Plugin, Host Plugin Package, Link Listener and Shell terms, ADR 0004 is rewritten for plugins, built-ins, `.vdplugin` packages, standalone plugins, two CLIs and several links, the new public `vitadeck-host-control` skill and updates to the init, manual-testing, upload and build skills teach agents the feature, `AGENTS.md` lists the new code and skill, and `agent-check.sh fast` runs `test:host-companion`.

[groups/docs.md](groups/docs.md)