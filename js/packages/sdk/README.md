# @vitadeck/sdk

Build tooling and the VitaDeck Runtime API for Deck Apps.

## Requirements

- Node.js 22+
- React **18.3.x** (peer dependency)

## Install

```bash
npm install @vitadeck/sdk react
```

pnpm and Yarn work the same (`pnpm add`, `yarn add`).

## Scaffold a project

```bash
npx vitadeck create <project-name>
```

Or, with a local install: `vitadeck create <project-name>`.

## Build

From the root of a Deck App (with `vitadeck.config.json`):

```bash
npx vitadeck build
```

`vitadeck` with no subcommand runs **build**. Use `vitadeck build --no-zip` to skip the `.zip` archive.

## Links

- **Repository:** https://github.com/matemolnar8/vitadeck (monorepo; this package is under `js/packages/sdk`.)
- **Issues:** https://github.com/matemolnar8/vitadeck/issues
