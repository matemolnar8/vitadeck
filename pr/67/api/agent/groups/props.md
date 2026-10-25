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

Review concern 05 of 05: ButtonProps and ImageProps from host prop types (`props`)

The why:

`ButtonProps` duplicated `VitaButtonProps` and `ImageProps` still allowed `key`, unlike Rect, Text and Scroll ([#56](source:s1)).

The what:

`ButtonProps` is `Omit<VitaButtonProps, "key">` and `ImageProps` uses a distributive `OmitKey` so the width/height union survives.

Look for:
- Subtle: a plain `Omit<VitaImageProps, "key">` would accept `{ x, y, image }` with neither `width` nor `height`; `OmitKey` keeps rejecting it.

Hunk refs for this concern:
- js/packages/sdk/src/index.tsx