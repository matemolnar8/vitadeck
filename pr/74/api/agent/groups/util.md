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

Review concern 01 of 07: util module and caller switch (`util`)

The why:

Gives every module [one copy of each helper](source:s1) instead of diverging static copies.

The what:

`src/core/util.c` defines the `vd_` helpers once, and every former owner, including `package_manifest_harness`, deletes its static copy and calls the shared one.

Look for:
- Subtle: `vd_safe_package_name` keeps the `VD_PACKAGE_NAME_MAX` length check that only `package_manifest.c` had, so `package_library_remove` now rejects a 128-character `.vdapp` name it used to accept.
- Subtle: `vd_join_path` is `snprintf` with a checked return; on overflow it writes a truncated, terminated path and returns false, where `package_manifest.c`'s old copy left `out` untouched. Callers that ignore the result behave as before.

Hunk refs for this concern:
- src/core/util.h
- src/core/util.c
- CMakeLists.txt
- src/core/package_library.c
- src/core/package_manifest.c
- src/upload/archive.c
- src/upload/http_server.c
- src/ui/fonts.c@3+3
- src/ui/fonts.c@20+23
- src/ui/fonts.c@43+35
- src/ui/fonts.c@73+65
- src/ui/images.c
- tests/package_manifest_harness.c