Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 7539bc14c3481390b79fee499bc961194f4cd055` and `git rev-parse --verify d741b11fc23f7c1d6ffcad842e17f18c22c44f08` in this repository.
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

base (merge-base)  7539bc14c3481390b79fee499bc961194f4cd055

head               d741b11fc23f7c1d6ffcad842e17f18c22c44f08

Named refs at pin: origin/main ... d741b11f

Read the diff:

git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 d741b11fc23f7c1d6ffcad842e17f18c22c44f08

Commits:
- d741b11f build: configure host builds through CMake presets
- da535645 build: configure host builds with Ninja
- 56d96fa2 build: rebuild JS when an example's build script changes
- 9ffb5d4a build: compare real paths in the in-source build guard
- d821d043 build: rebuild JS for vitadeck and on scaffold edits, refuse in-source builds
- f779b22e build: one parallel run target, JS as a real dependency

Sources:
- ticket Máté's request Collapse harness sources into vitadeck_core, make JS a real dependency of vitadeck/run/stage_examples, replace js_copy with configure-time symlinks, update every surface and doc in place.
- transcript Máté in Claude Code · Oct 28 Use Ninja; then add CMake presets so the configure flags need no remembering.

The title:

One parallel run target, JS as a real dependency

The why:

The everyday make js run loop took about 11 s after a one-file C edit while compiling and linking took about 1 s; the rest was an unconditional pnpm build, a 25 MB copy on every build, and a nested cmake --build that disabled the make jobserver.

The what (medium):

The host CMake build compiles every source once into a vitadeck_core static library, runs pnpm build only when JS inputs change, and links js/dist/runtime and assets/ into the build tree instead of copying them on every build.

Look for:
- The request lets JS be a stamp or an output; this uses the real output js/dist/runtime/runtime.js, so cleaning a host build dir deletes that file from the source tree.
- Subtle: an existing Make build dir now fails `cmake --preset default` with CMake's generator-mismatch error, so `agent-check.sh` and `bench.sh` stop on an old `out/` until it is removed.
- Subtle: on Linux `agent-check.sh` no longer honors `CC`/`CXX`; the `linux` preset caches GCC, and `-DCMAKE_C_COMPILER=...` overrides it.

## Review concerns

### 01 vitadeck_core library and harnesses (`core`)

The app's sources minus main.c become one static library that the app and every harness link, which collapses the per-harness source lists into a foreach.

[groups/core.md](groups/core.md)

### 02 JS as a dependency, links instead of copies (`js`)

The removed js_build, js_copy, and nested js targets are replaced by a js target over a custom command that vitadeck, stage_examples, run, and the smoke harnesses depend on on the host.

Depends on:
- 01 vitadeck_core library and harnesses (`core`)

[groups/js.md](groups/js.md)

### 03 Ninja through CMake presets (`ninja`)

`CMakePresets.json` names the Ninja configure (`default`) and its GCC variant (`linux`), and every script and CI job that configures a host build calls a preset; the smoke image installs `ninja-build`, and `run` joins the console pool.

Depends on:
- 02 JS as a dependency, links instead of copies (`js`)

[groups/ninja.md](groups/ninja.md)

### 04 Docs, skills, and bench (`docs`)

The docs now say the host build reruns pnpm itself, is configured with `cmake --preset default`, and loops on `ninja -C out run`, while the Vita build still needs a manual pnpm build.

Depends on:
- 02 JS as a dependency, links instead of copies (`js`)

[groups/docs.md](groups/docs.md)