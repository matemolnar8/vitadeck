Answer questions about this git change.

## Steps

When no question follows this paste, explain this change.

1. Resolve the pinned SHAs.
   Run `git rev-parse --verify 7539bc14c3481390b79fee499bc961194f4cd055` and `git rev-parse --verify 56d96fa218b0f5a6d86a6609da45bd9d51f36b7a` in this repository.
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

head               56d96fa218b0f5a6d86a6609da45bd9d51f36b7a

Named refs at pin: origin/main ... HEAD

Read the diff:

git diff --find-renames 7539bc14c3481390b79fee499bc961194f4cd055 56d96fa218b0f5a6d86a6609da45bd9d51f36b7a

Commits:
- 56d96fa2 build: rebuild JS when an example's build script changes
- 9ffb5d4a build: compare real paths in the in-source build guard
- d821d043 build: rebuild JS for vitadeck and on scaffold edits, refuse in-source builds
- f779b22e build: one parallel run target, JS as a real dependency

Sources:
- ticket Máté's request Collapse harness sources into vitadeck_core, make JS a real dependency of vitadeck/run/stage_examples, replace js_copy with configure-time symlinks, update every surface and doc in place.

The title:

One parallel run target, JS as a real dependency

The why:

The everyday make js run loop took about 11 s after a one-file C edit while compiling and linking took about 1 s; the rest was an unconditional pnpm build, a 25 MB copy on every build, and a nested cmake --build that disabled the make jobserver.

The what (medium):

The host CMake build compiles every source once into a vitadeck_core static library, runs pnpm build only when JS inputs change, and links js/dist/runtime and assets/ into the build tree instead of copying them on every build.

Look for:
- Optional item 4 (CMakePresets.json, Ninja) is not in this diff; the request said to ask in the handoff.
- The request lets JS be a stamp or an output; this uses the real output js/dist/runtime/runtime.js, so make clean in a host build dir deletes that file from the source tree.

## Review concerns

### 01 vitadeck_core library and harnesses (`core`)

The app's sources minus main.c become one static library that the app and every harness link, which collapses the per-harness source lists into a foreach.

[groups/core.md](groups/core.md)

### 02 JS as a dependency, links instead of copies (`js`)

The removed js_build, js_copy, and nested js targets are replaced by a js target over a custom command that vitadeck, stage_examples, run, and the smoke harnesses depend on on the host.

Depends on:
- 01 vitadeck_core library and harnesses (`core`)

[groups/js.md](groups/js.md)

### 03 Docs, skills, and bench (`docs`)

The docs now say the host build reruns pnpm itself while the Vita build still needs a manual pnpm build, and bench.sh builds the host in parallel.

Depends on:
- 02 JS as a dependency, links instead of copies (`js`)

[groups/docs.md](groups/docs.md)