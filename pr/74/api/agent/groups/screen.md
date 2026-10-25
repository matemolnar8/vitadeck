Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify b7df893f29a6ec038ac778734faa2ee18f88c214` and `git rev-parse --verify 06341f44bca0e9643f246ebca9f9f173697c5600` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames b7df893f29a6ec038ac778734faa2ee18f88c214 06341f44bca0e9643f246ebca9f9f173697c5600 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  b7df893f29a6ec038ac778734faa2ee18f88c214

head               06341f44bca0e9643f246ebca9f9f173697c5600

Named refs at pin: b7df893f29a6ec038ac778734faa2ee18f88c214 ... 06341f44bca0e9643f246ebca9f9f173697c5600

Read the diff:

git diff --find-renames b7df893f29a6ec038ac778734faa2ee18f88c214 06341f44bca0e9643f246ebca9f9f173697c5600

Review concern 05 of 07: One screen-size constant (`screen`)

The why:

Screen size [lives in both `shell.c` and `bootstrap.h`](source:s1).

The what:

`shell.c` includes `core/bootstrap.h` and uses `VD_SCREEN_WIDTH`/`VD_SCREEN_HEIGHT` in place of its own copies.

Hunk refs for this concern:
- src/shell/shell.c@3+3
- src/shell/shell.c@19+17
- src/shell/shell.c@32+30
- src/shell/shell.c@336+331
- src/shell/shell.c@379+374
- src/shell/shell.c@439+434
- src/shell/shell.c@477+472