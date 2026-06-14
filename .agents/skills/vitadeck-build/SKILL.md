---
name: vitadeck-build
description: Builds the JS runtime (pnpm in js/), then vitadeck for the host (CMake + raylib) and for PSVita in the Vitasdk Docker Compose service. Use when editing vitadeck C/native or JS/deck-app code, verifying desktop and Vita targets, producing out/vitadeck or out-vita/vitadeck.vpk, or when the user mentions macOS build, PSVita, VPK, VITASDK, docker-compose vitasdk, or js/dist.
---

# vitadeck dual-target build

## Prerequisites

**JavaScript workspace** (run before native builds; CMake copies `js/dist/runtime` into `<build>/js/`):

- Node.js 22+ (Rolldown uses Node 22+ APIs)
- Dependencies once: `pnpm --dir js install`
- Produce runtime bundle and example `.vdapp` packages: `pnpm --dir js build` → writes `js/dist/runtime/runtime.js` and `js/examples/*/dist/*.vdapp`
- Typecheck: `pnpm --dir js tsc`; watch dev: `pnpm --dir js dev`; lint/format: `pnpm --dir js lint` / `pnpm --dir js format`

**Host (current machine)** — e.g. macOS:

- CMake 3.10+
- [raylib](https://www.raylib.com/) via Homebrew: `brew install raylib`
- CURL and OpenSSL (typical on macOS; CMake `find_package` must succeed)

**PSVita** — no local Vitasdk install required:

- Docker with Compose
- Repo root must contain `docker-compose.yml` (service `vitasdk` mounts the repo at `/build/git`)

## Quick start

From the **vitadeck repository root**, in order:

**1. JS** (stages runtime under `js/dist/runtime/`; examples build to `js/examples/*/dist/*.vdapp`):

```sh
pnpm --dir js install
pnpm --dir js build
```

**2. Host** (executable `out/vitadeck`):

```sh
cmake -S . -B out && cmake --build out
```

**3. PSVita** (artifacts under `out-vita/`, including `vitadeck.vpk` when packaging is enabled):

```sh
docker-compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"
```

## Workflows

### After JS, TSX, or deck-app changes

1. `pnpm --dir js build` (and `pnpm --dir js tsc` when checking types).
2. Rebuild host: `cmake --build out` (or full configure if CMake changed).
3. Rebuild Vita: `docker-compose run --rm vitasdk bash -lc "cmake --build out-vita"` (or full configure in the container if CMake changed).

### After C or CMake changes

- C sources under `src/` use **clang-format** (`.clang-format` at repo root; editor: **clangd** with format-on-save). Check: `./scripts/clang-format.sh check`; apply: `./scripts/clang-format.sh format`. Run `cmake -S . -B out` so `out/compile_commands.json` exists for clangd.

1. Ensure `js/dist/` is current — run **`pnpm --dir js build`** if JS sources changed or after a clean.
2. Rebuild host: `cmake --build out` (or full configure line if `CMakeLists.txt` changed).
3. Rebuild Vita: `docker-compose run --rm vitasdk bash -lc "cmake --build out-vita"` (or full configure inside the container if CMake changed).

### Clean or switch generators

- Host: remove `out/` and re-run JS step if needed, then configure + build.
- Vita: remove `out-vita/` inside the same `docker-compose run ... bash -lc "..."` pattern, then configure and build again.

### Verify both targets

Run **JS build**, then the host quick start, then the Vita quick start. Fix compile errors on each target separately — `CMakeLists.txt` branches on `VITASDK` for Vita vs raylib for host.

## Notes

- In the container, `VITASDK` is set by the image; the project picks the Vita toolchain when that env var is present.
- Host builds use `out/`; Vita builds use `out-vita/` on the **host** filesystem (bind-mounted from the container).
- CMake can invoke JS via the `js_build` target; for agent workflows, treat **`pnpm --dir js build` before native builds** as the normal prerequisite so `js/dist/` is explicit and up to date.
- The **`vitadeck`** CLI is a compiled **`js/packages/sdk/dist/cli.mjs`** (plus **`dist/templates/`**); **`pnpm --dir js install`** runs the workspace **`prepare`** script, which builds **`@vitadeck/sdk`**. See `.cursor/rules/js-runtime-build.mdc`.
