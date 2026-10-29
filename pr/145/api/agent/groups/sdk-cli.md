Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 a90a3cf6f48b5e0e53fdcef8946f89a86bcb85a0 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 04 of 06: SDK CLI: build writes the .vdplugin package and zip, create --host-plugin scaffolds it (`sdk-cli`)

The why:

Authors [keep the plugin and the Deck App in one repo, with scaffold help](source:s1).

The what:

`hostPlugin: { entry, files? }` in `vitadeck.config.json` makes `build` bundle the entry with rolldown into a staged `<slug>.vdplugin` folder with `manifest.json` and a copied `files/`, swap it into `outDir` and zip it like the `.vdapp`, while `create --host-plugin` lays the `templates/host-plugin` overlay over the scaffold, merging its JSON files one level deep.

Look for:
- A failed plugin build leaves the previous `<slug>.vdplugin` in place: the package is written to `<slug>.vdplugin.tmp` and renamed over the old folder only after the bundle, `files/` copy and manifest succeed.
- `hostPlugin.files` must be a folder inside the project other than its root; links in it are copied as the files they point to, so the folder and the zip hold the same files.
- The `.vdplugin.zip` is written beside the `.vdapp.zip`; `vitadeck dev` rebuilds the package every cycle but only uploads the `.vdapp.zip`, and the scaffold's `host` script runs the companion with `--plugin ./dist`.
- The new root devDependency `@vitadeck/sdk` in `js/package.json` exists so the overlay template, which sits outside any workspace package, resolves `@vitadeck/sdk` for the SDK's `tsc` and type-aware lint.

Depends on:
- 01 SDK: plugin implementation API and the systemStats registry (`sdk-contract`)

Hunk refs for this concern:
- js/packages/sdk/cli/cli.ts
- js/packages/sdk/cli/copy-templates.ts
- js/packages/sdk/cli/templates/host-plugin/host/companion.ts
- js/packages/sdk/cli/templates/host-plugin/host/plugin.ts
- js/packages/sdk/cli/templates/host-plugin/host/tsconfig.json
- js/packages/sdk/cli/templates/host-plugin/package.json
- js/packages/sdk/cli/templates/host-plugin/src/App.tsx
- js/packages/sdk/cli/templates/host-plugin/vitadeck.config.json
- js/packages/sdk/cli/test-project.ts
- js/packages/sdk/cli/cli.test.ts
- js/package.json@24+24