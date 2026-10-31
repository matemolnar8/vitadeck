Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify ae5083caa9370b5f5985de1de0aa147a1afecc2e` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f ae5083caa9370b5f5985de1de0aa147a1afecc2e -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 02 of 09: Host Companion: vitadeck-host-companion, built-ins and plugin packages (`companion`)

The why:

Slice B: the long-lived listener on the computer that performs the link and runs plugin commands ([#110](source:s2)), installable on its own and loading plugins from `.vdplugin` packages ([session](source:s5)).

The what:

`cli.ts` runs `start`, which serves `POST /command` from `server.ts` over `core`, the built-in `systemStats` and every `.vdplugin` package `plugin.ts` loads from `~/.vitadeck/plugins/` or `--plugin` (reloading them on change through `setPlugins`), `install`/`remove`, which unpack and delete those packages, and `link`, which POSTs the link payload from `link.ts` to the Vita LAN Listener (port 8787 by default) and records the Vita in `config.ts`; every `start` relinks the recorded Vitas with `relink: true`, one at a time, retrying an unanswering one every 30 s and dropping one that answers `unknown_link`; `rolldown.config.mjs` bundles it with the SDK into one dependency-free `vitadeck-host-companion` bin. `notify` sends a `core` notice, `core.events` and `core.notify` live in `core.ts`, and the server starts plugin lifecycles once the port is bound and stops them on reload or close.

Look for:
- Subtle: `HostCommandError` is matched by `name` and wire `code`, not `instanceof`, because each package bundles its own copy of the class.
- A package's entry is imported as `<file URL>?v=<sha256 of the entry>`, so a rebuilt entry loads fresh but a rebuild that changes only `files/` keeps the cached module; plugins read `files/` per call.
- `install` stages into `.install-*` inside the plugins folder, validates, then renames to `<id>.vdplugin`; the extractor refuses several roots, backslashes, empty segments and `..`, ignores `__MACOSX/`, and keeps no file modes.
- Paths resolve against `INIT_CWD` only under the repo's own `host-companion` script; elsewhere against the working directory.
- On macOS `systemStats` counts free, inactive and speculative `vm_stat` pages as available; elsewhere it uses `os.freemem()`, which reads `MemAvailable` on Linux.
- `core.events` answers `unknown_plugin` for a plugin the companion has not loaded, so a Deck App watching a missing plugin finds out instead of waiting forever.
- A relink always targets port 8787 on the stored Vita IP, because the config keeps IPs only (they double as the answer allowlist); a Vita whose listener fell forward to another port needs `link` again.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

Hunk refs for this concern:
- js/packages/host-companion/package.json
- js/packages/host-companion/tsconfig.json
- js/packages/host-companion/src/cli.ts
- js/packages/host-companion/src/config.ts
- js/packages/host-companion/src/core.ts
- js/packages/host-companion/src/link.ts
- js/packages/host-companion/src/plugin.ts
- js/packages/host-companion/src/server.ts
- js/packages/host-companion/test/fixtures/greeter-plugin.ts
- js/packages/host-companion/test/link.test.ts
- js/packages/host-companion/test/server.test.ts
- js/packages/host-companion/README.md
- js/package.json
- .gitignore
- js/packages/host-companion/rolldown.config.mjs
- js/packages/host-companion/src/system-stats.ts
- js/packages/host-companion/test/cli.test.ts
- js/packages/host-companion/test/fixtures/greeting.cjs
- js/packages/host-companion/test/fixtures/greeting.d.cts
- js/packages/host-companion/test/fixtures/plugin-package.ts