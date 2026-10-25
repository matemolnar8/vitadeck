Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 24d9e5b7598d818b2bf067f3e2d5bfc70360c4ea` and `git rev-parse --verify 2763e8744697c1a6bd81f31c327ce0cfb09ade44` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 24d9e5b7598d818b2bf067f3e2d5bfc70360c4ea 2763e8744697c1a6bd81f31c327ce0cfb09ade44 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
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

Review concern 01 of 05: One declaration per native global (`globals`)

The why:

The runtime's copy of the author globals had drifted from the SDK's ([#56](source:s1)).

The what:

`runtime-native.d.ts` references `@vitadeck/sdk/deck-app-env` for console, timers, `getTime` and `fetch`, and `fetch.ts` drops its copies of the `Vita*` fetch types in favour of those globals.

Look for:
- Subtle: the runtime now sees `console.info`/`console.warn` and `setTimeout`/`clearTimeout`, which `log.c` and `timeout.c` always registered; the SDK file the authors get is unchanged.

Hunk refs for this concern:
- js/packages/runtime/src/runtime-native.d.ts
- js/packages/runtime/src/fetch.ts