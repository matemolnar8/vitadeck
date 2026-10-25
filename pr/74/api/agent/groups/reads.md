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

Review concern 02 of 07: Checked file reads (`reads`)

The why:

The old readers [malloc `len + 1` with a possibly negative `len`](source:s1).

The what:

`vd_read_file` fails on any `fseek`, `ftell`, or short `fread` error, and `jslib.c` and `js_runtime.c` replace their unchecked readers with it, while `archive.c`'s arena reader gets the same `fseek` checks.

Look for:
- A file that shrinks between `ftell` and `fread` now returns NULL instead of a partial, NUL-terminated buffer; `nativeReadTextFile` throws "Could not read file" for it.
- `vd_mkdir_p` and `vd_mkdir_parent` return false for a path of 512+ bytes instead of creating a truncated directory.

Depends on:
- 01 util module and caller switch (`util`)

Hunk refs for this concern:
- src/core/util.c
- src/jslib/jslib.c
- src/core/js_runtime.c@5+5
- src/core/js_runtime.c@41+42
- src/core/js_runtime.c@85+67
- src/upload/archive.c@32+25