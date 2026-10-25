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

Review concern 02 of 05: VITADECK_SANITIZE and the sanitized CI run (`sanitize`)

The why:

The ticket asks for a `VITADECK_SANITIZE` option used by the host CTest job ([#53](source:s1)).

The what:

The CMake option adds `-fsanitize=` to every host target, `smoke-docker.sh ctest` builds and runs all harnesses with it, and CI calls that mode as a second step after the golden smoke run.

Look for:
- `quickjs` builds with `-fno-sanitize=undefined`, so UBSan never reports inside the interpreter; ASan still instruments it.
- `-fno-sanitize-recover=all` makes the first UBSan report abort the test, so a new UB in `src/` fails CI instead of only printing.
- `tests/lsan.supp` suppresses any leak with `libLLVM` or `glfwCreateWindow` on its stack; only `smoke_harness` gets it, and no `src/` code runs beneath `glfwCreateWindow`, so a leak allocated by VitaDeck code still fails the run.

Depends on:
- 01 -Wall -Wextra and the fallout (`warnings`)

Hunk refs for this concern:
- CMakeLists.txt@10+10
- CMakeLists.txt@21+32
- CMakeLists.txt@89+106
- CMakeLists.txt@249+281
- .github/workflows/ci.yml@120+121
- .gitignore
- scripts/agent-check.sh
- scripts/smoke-docker.sh
- tests/lsan.supp