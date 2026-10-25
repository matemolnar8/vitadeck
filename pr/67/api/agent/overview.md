Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 24d9e5b7598d818b2bf067f3e2d5bfc70360c4ea` and `git rev-parse --verify 2763e8744697c1a6bd81f31c327ce0cfb09ade44` in this repository.
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

base (merge-base)  24d9e5b7598d818b2bf067f3e2d5bfc70360c4ea

head               2763e8744697c1a6bd81f31c327ce0cfb09ade44

Named refs at pin: 24d9e5b7598d818b2bf067f3e2d5bfc70360c4ea ... HEAD

Read the diff:

git diff --find-renames 24d9e5b7598d818b2bf067f3e2d5bfc70360c4ea 2763e8744697c1a6bd81f31c327ce0cfb09ade44

Commits:
- 2763e87 Merge origin/main into refactor/native-globals-single-source
- 1f5d954 refactor(sdk): derive ButtonProps and ImageProps from the host prop types
- 8816246 refactor(runtime): type the reconciler host config without a cast
- 0e2430e refactor(sdk): share the Deck App Package Manifest type with the runtime
- 5d771e8 test(js): check declared native globals against src/jslib registrations
- 82e3a5c refactor(runtime): layer native globals on the SDK's author globals

Sources:
- ticket #56 Deduplicate native global declarations, check them against src/jslib, reuse the manifest type, remove the host config cast, Omit key for Button/Image props.
  https://github.com/matemolnar8/vitadeck/issues/56

The title:

Single source for native globals and a typed reconciler host config

The why:

The native JS contract was declared three times and had drifted, nothing checked it against the C registrations, and the host config cast hid missing reconciler methods that only fail on device ([#56](source:s1)).

The what (medium):

Each native global is declared once, a script checks the declared names against `src/jslib`, the runtime reuses the SDK's manifest type, the reconciler host config is typed without a cast, and `ButtonProps`/`ImageProps` derive from the host prop types.

Look for:
- Done differently: [#56](source:s1) proposes one `.d.ts` in the runtime that the SDK narrows. The SDK is published and the runtime is private, so the SDK keeps the author globals in `deck-app-globals.d.ts` and `runtime-native.d.ts` references them and adds only the runtime-only contract.
- Done differently: [#56](source:s1) calls the manifest checks in `main.tsx` a re-validation. The checks stay; only the duplicate type goes, because the persisted Active Deck App is not re-validated natively at startup.

## Review concerns

### 01 One declaration per native global (`globals`)

`runtime-native.d.ts` references `@vitadeck/sdk/deck-app-env` for console, timers, `getTime` and `fetch`, and `fetch.ts` drops its copies of the `Vita*` fetch types in favour of those globals.

[groups/globals.md](groups/globals.md)

### 02 Native globals check in CI (`check`)

`check-native-globals.ts` regex-reads `js_set_global_function` names and the declared functions, runs as `test:native-globals` in CI, and is typechecked by the root `tsc` script through `scripts/tsconfig.json`.

Depends on:
- 01 One declaration per native global (`globals`)

[groups/check.md](groups/check.md)

### 03 Shared Deck App Package Manifest type (`manifest`)

`DeckAppPackageManifest` lives in the SDK, the CLI's written manifest `satisfies` it, and `main.tsx` imports it from `@vitadeck/sdk/internal`.

[groups/manifest.md](groups/manifest.md)

### 04 Host config without the unknown cast (`host-config`)

`rawHostConfig` now `satisfies VitadeckHostConfig`, so the ten required members it lacked are implemented and `Reconciler(hostConfig)` needs no cast.

[groups/host-config.md](groups/host-config.md)

### 05 ButtonProps and ImageProps from host prop types (`props`)

`ButtonProps` is `Omit<VitaButtonProps, "key">` and `ImageProps` uses a distributive `OmitKey` so the width/height union survives.

[groups/props.md](groups/props.md)