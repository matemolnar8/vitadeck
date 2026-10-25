Answer questions about this review concern.

## Steps

When no question follows this paste, explain this review concern.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` and `git rev-parse --verify 4d6d7c5a7816464240a173bb7185dcb1c8090429` in this repository.
   Done when both objects exist.

2. Load the hunks.
   A hunk ref is a pointer into the live git diff at the pinned SHAs.
   For each hunk ref, run `git diff --find-renames 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 4d6d7c5a7816464240a173bb7185dcb1c8090429 -- <path>` and keep the hunk whose header matches the @@ range.
   Done when every hunk ref has a matching live hunk.

3. Answer from live git.
   Read those hunks. Use the why and the what as interpretation. Live git wins when they disagree.
   When you show code, quote the live git lines.
   Done when the answer quotes the live code.

## Pin

Repository: vitadeck
Origin: git@github.com:matemolnar8/vitadeck.git

base (merge-base)  5cbf7dacf365fc2a6999c53b263160e1c53db5d8

head               4d6d7c5a7816464240a173bb7185dcb1c8090429

Named refs at pin: origin/perf/dirty-flag-and-caches ... origin/build/warnings-sanitizers-js-limits

Read the diff:

git diff --find-renames 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 4d6d7c5a7816464240a173bb7185dcb1c8090429

Review concern 01 of 05: -Wall -Wextra and the fallout (`warnings`)

The why:

The ticket asks for `-Wall -Wextra` with the fallout fixed ([#53](source:s1)).

The what:

`CMakeLists.txt` adds the warning flags globally while moving vendored and third-party headers to `-isystem` and building `quickjs` with `-w`, and the few warnings left in `src/` are fixed at their sites.

Look for:
- OpenSSL and host raylib include dirs moved from `-I` to `-isystem` alongside `vendor/`; the Vita SDK dirs stay `-I` so the toolchain's own system-dir order is untouched.

Hunk refs for this concern:
- CMakeLists.txt@10+10
- CMakeLists.txt@61+77
- CMakeLists.txt@70+89
- CMakeLists.txt@89+106
- .github/workflows/ci.yml@80+80
- src/jslib/timeout.c
- src/jslib/fetch.c@308+320
- src/jslib/fetch.c@323+335
- src/platform/thread_vita.c@18+16
- src/ui/render.c