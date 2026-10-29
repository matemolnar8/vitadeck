Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0` in this repository.
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

base (merge-base)  04760101d68405a97b39f799f3f4f9181fa18778

head               a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0

Named refs at pin: 04760101 ... a90a3cf6

Read the diff:

git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0

Commits:
- a90a3cf6 fix(sdk): build the plugin package beside the old one and swap it in
- e069a2f5 fix(host-control): a broken plugin build keeps the last good package
- a601a6ca feat(host-control): a Deck App's plugin ships as a .vdplugin package and zip
- 96448b7b fix(host-companion): --plugin takes a directory, so the scaffold survives a renamed app
- c0498393 fix(host-companion): allow plugin ids as long as a package slug
- 445f5ef3 fix(host-companion): rename the plugin's module copy into place
- 966550e1 fix(host-companion): install renames the plugin into place
- 4139145b fix(host-companion): load plugins from file copies, resolve paths from the cwd
- 148fceef feat(host-control): built-in plugins, .vdplugin files, vitadeck-host-companion

Sources:
- transcript Claude Code session · Oct 29 Máté asks for same-repo plugins, built-ins, easier loading (maybe a .vdplugin), a vitadeck-host-companion bin, and a look at a single vitadeck CLI, then a compromise.
- pr PR #145 Built-ins first, a .vdplugin per Deck App, install/remove and hot reload, two CLIs; stacked on #144.
  https://github.com/matemolnar8/vitadeck/pull/145
- pr-comment chatgpt-codex-connector on PR #145
  https://github.com/matemolnar8/vitadeck/pull/145#discussion_r4408848004

The title:

Deck Apps ship their own host plugin as a .vdplugin package

The why:

Máté asked that [authors keep the plugin and the Deck App in one repo, without an npm package](source:s1), that [built-in plugins cover the use cases with custom ones only when needed](source:s1), that plugin loading get simpler, and that [the global CLI be called vitadeck-host-companion](source:s1).

The what (large):

Host Control Plugins move to two kinds: built-ins (`core`, `systemStats`) inside the Host Companion, and a Deck App's own plugin, which `vitadeck build` turns from `hostPlugin` into a `<slug>.vdplugin` package folder and `.vdplugin.zip` that `vitadeck-host-companion install` unpacks and hot-reloads.

Look for:
- Done as asked later in [the session](source:s1): a `.vdplugin` is a vdapp-like package (`manifest.json`, bundled `companion.mjs`, optional `files/`) with a `.vdplugin.zip` whose one top-level entry is that folder; the first commits shipped a single JS file and the format changed in a601a6ca.
- Considered, not chosen: [the request](source:s1) floated a single `vitadeck` CLI. The companion keeps its own bin, `vitadeck-host-companion`, because a second bin in `@vitadeck/sdk` breaks `npx @vitadeck/sdk` and users would install Babel and rolldown to run a server.
- Not true at head: `@vitadeck/host-companion` is still `private: true`, so a project from `create --host-plugin` cannot `npm install` its `@vitadeck/host-companion` devDependency until the package is published; [PR #145](source:s2) tests with packed tarballs instead.

## Review concerns

### 01 SDK: plugin implementation API and the systemStats registry (`sdk-contract`)

`implementHostPlugin`, `HostCommandError`, `ImplementedHostPlugin` and the `HostPluginManifest` contract move into the SDK's `plugin.ts`, and the `systemStatsPlugin` registry moves from `js/host-plugins` into `system-stats.ts`, all re-exported from `index.ts`.

[groups/sdk-contract.md](groups/sdk-contract.md)

### 02 Companion: built-ins and loading Host Plugin Packages (`companion-loading`)

`loadHostPluginPackage` reads a `.vdplugin` folder's manifest and imports its entry by file URL with a content-hash query, `extractHostPluginArchive` unpacks a `.vdplugin.zip`, and the server's new `setPlugins` swaps every plugin but `core`, while `systemStats` becomes a built-in implementation beside `core`.

Depends on:
- 01 SDK: plugin implementation API and the systemStats registry (`sdk-contract`)

[groups/companion-loading.md](groups/companion-loading.md)

### 03 Companion CLI: vitadeck-host-companion install, remove and hot reload (`companion-cli`)

`start` loads the built-ins, every package in `~/.vitadeck/plugins/` and each `--plugin` package or folder of packages, watches those folders recursively and calls `setPlugins` after changes, while `install` stages an archive or folder in `.install-*`, validates it and renames it to `<id>.vdplugin`, and `remove` deletes that folder.

Depends on:
- 02 Companion: built-ins and loading Host Plugin Packages (`companion-loading`)

[groups/companion-cli.md](groups/companion-cli.md)

### 04 SDK CLI: build writes the .vdplugin package and zip, create --host-plugin scaffolds it (`sdk-cli`)

`hostPlugin: { entry, files? }` in `vitadeck.config.json` makes `build` bundle the entry with rolldown into a staged `<slug>.vdplugin` folder with `manifest.json` and a copied `files/`, swap it into `outDir` and zip it like the `.vdapp`, while `create --host-plugin` lays the `templates/host-plugin` overlay over the scaffold, merging its JSON files one level deep.

Depends on:
- 01 SDK: plugin implementation API and the systemStats registry (`sdk-contract`)

[groups/sdk-cli.md](groups/sdk-cli.md)

### 05 Companion package, workspace and the demo app (`packaging`)

The companion's `package.json` gains the `vitadeck-host-companion` bin and a rolldown build that inlines the SDK so it has no dependencies, `js/host-plugins` leaves the workspace, and the demo Deck App and Shell hint switch to the built-in registry and the new command name.

Depends on:
- 03 Companion CLI: vitadeck-host-companion install, remove and hot reload (`companion-cli`)

[groups/packaging.md](groups/packaging.md)

### 06 Docs: ADR 0004, glossary and READMEs (`docs`)

ADR 0004's plugin section is rewritten for built-ins, Host Plugin Files and the two CLIs, CONTEXT.md adds **Host Plugin File**, and the companion, SDK and demo READMEs switch to the published install and the `--host-plugin` workflow.

Depends on:
- 05 Companion package, workspace and the demo app (`packaging`)

[groups/docs.md](groups/docs.md)