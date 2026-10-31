Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify a447e3b960dc1c5434b9fbc2f45c112b1c630b1f` and `git rev-parse --verify 70e9ba2bb5603104bab02541ecf29a039796344b` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 70e9ba2bb5603104bab02541ecf29a039796344b -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  a447e3b960dc1c5434b9fbc2f45c112b1c630b1f

head               70e9ba2bb5603104bab02541ecf29a039796344b

Named refs at pin: a447e3b9 ... 70e9ba2b

Read the diff:

git diff --find-renames a447e3b960dc1c5434b9fbc2f45c112b1c630b1f 70e9ba2bb5603104bab02541ecf29a039796344b

Review concern 04 of 09: SDK CLI: build Host Plugin packages, with or without a Deck App (`sdk-cli`)

The why:

Authors keep a plugin in a Deck App's repo or on its own, and ship it as a `.vdplugin` package like the `.vdapp` ([session](source:s5)).

The what:

`hostPlugin: { entry, files? }` in `vitadeck.config.json` makes `build` bundle the entry into a staged `<slug>.vdplugin` folder with `manifest.json` and `files/`, swap it in and zip it, `entry` becomes optional so a plugin-only project builds and `dev`s alone, and `create --host-plugin` / `--host-plugin-only` scaffold the two layouts from the `host-plugin` and `host-plugin-only` templates.

Look for:
- A failed plugin build leaves the previous `<slug>.vdplugin` in place: the package is written to `<slug>.vdplugin.tmp` and renamed over the old folder only after everything succeeds.
- `vitadeck dev` takes a Runtime Upload URL exactly when the project has a Deck App `entry`; a plugin-only project rebuilds on save without one.
- The standalone template takes `@vitadeck/sdk` as a peer and dev dependency and exports `./host/plugin.ts`, so a Deck App installing it from git or npm shares its own SDK; `copy-templates.ts` rewrites `workspace:*` in every template's dependency fields.
- The scaffolded plugin id is the project slug, so two projects scaffolded with the same name collide on one companion.

Depends on:
- 01 Wire contract and SDK client (`sdk-contract`)

Hunk refs for this concern:
- js/packages/sdk/cli/cli.test.ts@1+1
- js/packages/sdk/cli/cli.test.ts@61+71
- js/packages/sdk/cli/cli.ts@1+1
- js/packages/sdk/cli/cli.ts@7+7
- js/packages/sdk/cli/cli.ts@34+38
- js/packages/sdk/cli/cli.ts@102+107
- js/packages/sdk/cli/cli.ts@186+201
- js/packages/sdk/cli/cli.ts@232+252
- js/packages/sdk/cli/cli.ts@366+387
- js/packages/sdk/cli/cli.ts@450+573
- js/packages/sdk/cli/cli.ts@551+703
- js/packages/sdk/cli/cli.ts@565+725
- js/packages/sdk/cli/cli.ts@613+775
- js/packages/sdk/cli/cli.ts@628+790
- js/packages/sdk/cli/cli.ts@636+801
- js/packages/sdk/cli/cli.ts@657+833
- js/packages/sdk/cli/cli.ts@692+868
- js/packages/sdk/cli/copy-templates.ts@8+8
- js/packages/sdk/cli/copy-templates.ts@27+44
- js/packages/sdk/cli/templates/host-plugin-only/.gitignore
- js/packages/sdk/cli/templates/host-plugin-only/package.json
- js/packages/sdk/cli/templates/host-plugin-only/vitadeck.config.json
- js/packages/sdk/cli/templates/host-plugin/host/companion.ts
- js/packages/sdk/cli/templates/host-plugin/host/plugin.ts
- js/packages/sdk/cli/templates/host-plugin/host/tsconfig.json
- js/packages/sdk/cli/templates/host-plugin/package.json
- js/packages/sdk/cli/templates/host-plugin/src/App.tsx
- js/packages/sdk/cli/templates/host-plugin/vitadeck.config.json
- js/packages/sdk/cli/cli.ts@514+663
- js/packages/sdk/cli/dev.test.ts