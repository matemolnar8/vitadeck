Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 5cbf7dacf365fc2a6999c53b263160e1c53db5d8` and `git rev-parse --verify 56d626854fbd0253c445545cf51d4e49db77d014` in this repository.
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

base (merge-base)  5cbf7dacf365fc2a6999c53b263160e1c53db5d8

head               56d626854fbd0253c445545cf51d4e49db77d014

Named refs at pin: origin/perf/dirty-flag-and-caches ... build/warnings-sanitizers-js-limits

Read the diff:

git diff --find-renames 5cbf7dacf365fc2a6999c53b263160e1c53db5d8 56d626854fbd0253c445545cf51d4e49db77d014

Commits:
- 56d6268 fix(core): bound QuickJS stack and memory, cap fetch bodies
- 5138cb3 build: add VITADECK_SANITIZE and a sanitized CTest CI step
- 7a1546d build: enable -Wall -Wextra and a VITADECK_WERROR option

Sources:
- ticket #53 Enable -Wall -Wextra, add VITADECK_SANITIZE for host CTest, set QuickJS memory and stack limits sized for the Vita thread, cap fetch bodies, make the JS thread stack explicit.
  https://github.com/matemolnar8/vitadeck/issues/53
- pr PR #76 Implements #53 in three commits, stacked on #75.
  https://github.com/matemolnar8/vitadeck/pull/76

The title:

build: warnings, sanitizer option, QuickJS and fetch limits

The why:

QuickJS's default stack guard assumes more stack than the 256 KiB Vita thread has, and nothing bounds JS memory or `fetch` bodies ([#53](source:s1)).

The what (medium):

Turns on `-Wall -Wextra` with an opt-in `-Werror`, adds a host `VITADECK_SANITIZE` option run in CI, gives every `vd_thread` an explicit stack, bounds the QuickJS stack and heap, and caps `fetch` response bodies at 4 MiB.

Look for:
- The ticket also lists "no Debug config for Vita" as part of the problem ([#53](source:s1)); the Vita build still sets only `-O3` and this PR adds no Debug variant there.

## Review concerns

### 01 -Wall -Wextra and the fallout (`warnings`)

`CMakeLists.txt` adds the warning flags globally while moving vendored and third-party headers to `-isystem` and building `quickjs` with `-w`, and the few warnings left in `src/` are fixed at their sites.

[groups/warnings.md](groups/warnings.md)

### 02 VITADECK_SANITIZE and the sanitized CI run (`sanitize`)

The CMake option adds `-fsanitize=` to every host target, `smoke-docker.sh ctest` builds and runs all harnesses with it, and CI calls that mode as a second step after the golden smoke run.

Depends on:
- 01 -Wall -Wextra and the fallout (`warnings`)

[groups/sanitize.md](groups/sanitize.md)

### 03 Explicit thread stack and QuickJS limits (`limits`)

`VD_THREAD_STACK_SIZE` in `thread.h` sets every `vd_thread`'s stack on both platforms, and `js_thread_func` derives the QuickJS stack limit from it minus 64 KiB and caps the JS heap at 64 MiB.

[groups/limits.md](groups/limits.md)

### 04 fetch body cap (`fetch-cap`)

`fetch_write_cb` stops curl once the body would pass `FETCH_MAX_BODY_BYTES`, and `fetch_worker` turns that into a rejection with a fixed message that `CONTEXT.md` documents for authors.

[groups/fetch-cap.md](groups/fetch-cap.md)

### 05 js_limits harness (`harness`)

`tests/js_limits_harness.c` writes stand-in `js/runtime.js` files and runs them on the real JS thread, and CMake builds it twice, once forcing `VD_THREAD_STACK_SIZE` to the Vita value, on a source list shared with `smoke_harness`.

Depends on:
- 03 Explicit thread stack and QuickJS limits (`limits`)
- 04 fetch body cap (`fetch-cap`)

[groups/harness.md](groups/harness.md)