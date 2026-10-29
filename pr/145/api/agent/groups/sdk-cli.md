Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 04760101d68405a97b39f799f3f4f9181fa18778` and `git rev-parse --verify 96448b7b14c1ae72218b1b92b451d0ca6ed90434` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 96448b7b14c1ae72218b1b92b451d0ca6ed90434 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  04760101d68405a97b39f799f3f4f9181fa18778

head               96448b7b14c1ae72218b1b92b451d0ca6ed90434

Named refs at pin: 04760101 ... 96448b7b

Read the diff:

git diff --find-renames 04760101d68405a97b39f799f3f4f9181fa18778 96448b7b14c1ae72218b1b92b451d0ca6ed90434

Review concern 04 of 06: SDK CLI: build writes the .vdplugin, create --host-plugin scaffolds it (`sdk-cli`)

The why:

Authors [keep the plugin and the Deck App in one repo, with scaffold help](source:s1).

The what:

`hostPlugin` in `vitadeck.config.json` makes `build` bundle that entry with rolldown into `<outDir>/<slug>.vdplugin`, and `create --host-plugin` lays the `templates/host-plugin` overlay over the scaffold, merging its JSON files one level deep, with `copy-templates.ts` pinning the companion version.

Look for:
- The `.vdplugin` is written beside the `.vdapp.zip`, not inside it; `vitadeck dev` rebuilds it every cycle but only uploads the zip, and the scaffold's `host` script runs the companion with `--plugin ./dist`, so the file is picked up whatever the app's slug.
- The new root devDependency `@vitadeck/sdk` in `js/package.json` exists so the overlay template, which sits outside any workspace package, resolves `@vitadeck/sdk` for the SDK's `tsc` and type-aware lint.
- The scaffolded plugin id is the project slug, so two Deck Apps scaffolded with the same name collide on one companion.

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