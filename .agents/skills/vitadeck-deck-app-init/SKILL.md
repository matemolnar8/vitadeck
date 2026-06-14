---
name: vitadeck-deck-app-init
description: Scaffolds a standalone VitaDeck Deck App project with npx @vitadeck/sdk create outside the monorepo. Use when the user wants to start a new Deck App, initialize a deck app project, run vitadeck create, or set up @vitadeck/sdk from npm.
---

# Initialize a Deck App (standalone)

## What is a Deck App?

A **Deck App** is a user-authored interactive experience that runs on [VitaDeck](https://github.com/matemolnar8/vitadeck) on PlayStation Vita (or the desktop host). You write it in **React** against the **VitaDeck Runtime API** (`@vitadeck/sdk` components like `Screen`, `Text`, `Button`).

At build time, the SDK compiles your **Deck App Source Entry** (`src/App.tsx` by default) into a **Deck App Package** — a portable `.vdapp` folder (plus a `.vdapp.zip` archive for upload). The package contains your compiled app and metadata; VitaDeck supplies React and the runtime on device.

This workflow assumes a **standalone project** anywhere on disk, using the published npm package — not the VitaDeck monorepo.

## Prerequisites

- **Node.js 22+**
- npm, pnpm, or Bun
- [VitaDeck installed on Vita](https://github.com/matemolnar8/vitadeck/releases) (or desktop host) to run the built package

## Quick start

From the directory where the new project should live:

```sh
npx @vitadeck/sdk create my-deck-app
cd my-deck-app
npm install
```

Edit `src/App.tsx` — the default export is your **Deck App Component** (root UI for the full Vita display).

Build:

```sh
npx @vitadeck/sdk build
# or: npm run build
```

Outputs under `dist/` (from `vitadeck.config.json`):

- `dist/<slug>.vdapp/` — unpacked **Deck App Package**
- `dist/<slug>.vdapp.zip` — **Runtime Upload Archive** for LAN publish

The `<slug>` is derived from the project `name` in `vitadeck.config.json` (e.g. `my-deck-app` → `my-deck-app.vdapp`).

## What `create` generates

| File                    | Purpose                                                                |
| ----------------------- | ---------------------------------------------------------------------- |
| `vitadeck.config.json`  | Deck App manifest: `name`, `entry`, `outDir`                           |
| `package.json`          | Pins `@vitadeck/sdk` (`^` matching scaffold version), React **18.3.x** |
| `src/App.tsx`           | **Deck App Source Entry** — default-exported root component            |
| `src/vitadeck-env.d.ts` | TypeScript env for Deck App globals                                    |
| `tsconfig.json`         | Extends `@vitadeck/sdk/tsconfig`                                       |

Optional `vitadeck.config.json` fields: `version` (overrides `package.json` semver in the built manifest), `fonts` (named font files copied into the package).

## After scaffolding

1. **Typecheck:** `npx tsc` or `npm run tsc`
2. **Iterate:** edit `src/App.tsx`, rebuild with `npx @vitadeck/sdk build`
3. **Skip zip:** `npx @vitadeck/sdk build --no-zip` when only the `.vdapp` folder is needed
4. **Deploy to Vita:** upload `dist/*.vdapp.zip` via Runtime Upload — see [vitadeck-vdapp-upload](../vitadeck-vdapp-upload/SKILL.md)

## Requirements and constraints

- **React 18.3.x** — peer dependency; build fails if another major/minor is resolved
- **Entry** must default-export one React component; do not call VitaDeck registration APIs — the SDK generates that
- **Package version** in the built `manifest.json` comes from `package.json` `version` unless `vitadeck.config.json` sets `version`

## Troubleshooting

- **`Usage: vitadeck create <project-name>`** — pass a project folder name: `npx @vitadeck/sdk create my-app`
- **Target directory already exists** — pick a new name or remove the existing folder
- **React version error** — ensure `react` is `18.3.x` in `package.json`, then reinstall
- **Wrong context** — inside the VitaDeck monorepo, examples live under `js/examples/` and use workspace linking; this skill is for external projects only

## Links

- npm: [@vitadeck/sdk](https://www.npmjs.com/package/@vitadeck/sdk)
- SDK readme: [js/packages/sdk/README.md](https://github.com/matemolnar8/vitadeck/blob/main/js/packages/sdk/README.md)
