# @vitadeck/sdk

Build tooling and the VitaDeck Runtime API for Deck Apps.

## Requirements

- Node.js 22+
- React **18.3.x** (peer dependency)

## Install

```bash
npm install @vitadeck/sdk
```

pnpm and Bun work the same (`pnpm add`, `bun add`).

## Scaffold a project

```bash
npx @vitadeck/sdk create <project-name>
```

## Build

From the root of a Deck App (with `vitadeck.config.json`):

```bash
npx @vitadeck/sdk build
```

`npx @vitadeck/sdk` with no subcommand runs **build**. Use `npx @vitadeck/sdk build --no-zip` to skip the `.zip` archive.

## Links

- **Repository:** [https://github.com/matemolnar8/vitadeck](https://github.com/matemolnar8/vitadeck) (monorepo; this package is under `js/packages/sdk`.)
- **Issues:** [https://github.com/matemolnar8/vitadeck/issues](https://github.com/matemolnar8/vitadeck/issues)

