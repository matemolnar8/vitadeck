Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` in this repository.
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

head               04760101d68405a97b39f799f3f4f9181fa18778

Named refs at pin: a447e3b9 ... HEAD

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 04760101d68405a97b39f799f3f4f9181fa18778

Commits:
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

Host Control: let Deck Apps call linked computers through plugins

The why:

Máté asked for the three Host Control slices, shaped so later Deck Apps can send keys, read sensors and set controls, act like a Stream Deck, drive several machines on different OSes, and show a T3 Code dashboard ([session](source:s5)).

The what (very large):

Adds Host Control end to end: the `@vitadeck/sdk/host-control` client and plugin registry, the Host Companion CLI with a read-only `system-stats` plugin, the native link listener and link store with a Shell Host Control screen, and a `host-control` example Deck App.

Look for:
- [#109](source:s1) asks for flat command names and an internal `defineHostCommands`. Every request instead names a `plugin`, and `defineHostPlugin` and `command` are public, because the [session](source:s5) left the extension model open and [PR #144](source:s4) chose plugins.
- [#109](source:s1) and [#111](source:s3) name the getter `nativeGetHostControlUrl()` and one stored link in `host-control-link.json`. At head `nativeGetHostControlLinks()` returns a JSON array of up to 4 links in `host-control-links.json`, keyed by a new companion `id` in the link payload, so one Deck App can reach several machines ([session](source:s5)).
- [#109](source:s1) asks for a new `test:sdk` script. At head the SDK host-control tests run under the existing `test:cli` glob; no source gives a reason.
- [#110](source:s2) keeps one `vita` in the companion config. At head `vitas` is an array, since several Vitas can link one companion.
- [#111](source:s3) describes a `Forget link` action. At head Cross on a linked computer row forgets that one link at once.
- Credentials stay deferred, as ADR 0004 says, so the only plugins ([PR #144](source:s4)) read information; no acting command ships.

## Review concerns

### 01 Wire contract and SDK client (`sdk-contract`)

`createHostClient` in `client.ts` turns a `defineHostPlugin` registry from `plugin.ts` into typed methods that POST the `protocol.ts` envelope and map every failure to a `HostResult`, checked against the shared fixtures.

[groups/sdk-contract.md](groups/sdk-contract.md)

### 02 Host Companion CLI (`companion`)

`cli.ts` runs `start`, which serves `POST /command` from `server.ts` over the plugins `plugin.ts` loads, and `link`, which POSTs the link payload from `link.ts` and records the Vita in `config.ts`.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

[groups/companion.md](groups/companion.md)

### 03 system-stats plugin (`system-stats`)

`index.ts` is the registry a Deck App bundles and `companion.ts` implements its one `read` command with `node:os`, reading `vm_stat` on macOS.

Depends on:
- 02 Host Companion CLI (`companion`)

[groups/system-stats.md](groups/system-stats.md)

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

`App.tsx` polls `linkedHosts()` and each host's `systemStats.read`, lists the computers with live CPU, and shows the selected one's capabilities, gauges and an Echo round trip.

Depends on:
- 03 system-stats plugin (`system-stats`)

[groups/demo.md](groups/demo.md)

### 08 Glossary, ADR and checks (`docs`)

`CONTEXT.md` adds the Host Control Plugin, Link Listener and Shell terms, ADR 0004 is rewritten for plugins and several links, `AGENTS.md` lists the new code, and `agent-check.sh fast` runs `test:host-companion`.

[groups/docs.md](groups/docs.md)