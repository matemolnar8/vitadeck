Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d` and `git rev-parse --verify 3ad801f013a9deced2cca7d5928b544f9aa18750` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 3ad801f013a9deced2cca7d5928b544f9aa18750 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  34ef1c8e1b3eea604180cb158cd9bdffa48ed89d

head               3ad801f013a9deced2cca7d5928b544f9aa18750

Named refs at pin: 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d ... chore/docs-and-hygiene

Read the diff:

git diff --find-renames 34ef1c8e1b3eea604180cb158cd9bdffa48ed89d 3ad801f013a9deced2cca7d5928b544f9aa18750

Review concern 03 of 07: Delete the unused light theme (`theme`)

Part: Theme

The why:

[#59](source:s1) notes the theme is fixed to `dark` while `light` is exported.

The what:

`ThemeProvider` in `theme.tsx` serves one constant `Theme`, so `index.tsx` stops exporting `ThemeName` and the smoke fixture prints the literal `dark`.

Look for:
- Breaking: authors importing `ThemeName`, reading `theme.name`, or destructuring `themeName` from `useTheme()` get type errors; `useTheme().theme` colors are unchanged.
- The smoke label text stays `SDK / runtime / theme: dark` because `render.c` concatenates raw-text children, so the golden does not move.

Hunk refs for this concern:
- js/packages/sdk/src/theme.tsx
- js/packages/sdk/src/index.tsx
- js/examples/smoke/src/App.tsx