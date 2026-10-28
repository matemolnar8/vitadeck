---
name: vitadeck-build
description: Builds the JS runtime (pnpm in js/), then vitadeck for the host (CMake + raylib) and for PSVita in the Vitasdk Docker Compose service. Use when editing vitadeck C/native or JS/deck-app code, verifying desktop and Vita targets, producing out/vitadeck or out-vita/vitadeck.vpk, or when the user mentions macOS build, PSVita, VPK, VITASDK, docker compose vitasdk, or js/dist.
metadata:
  internal: true
---

# vitadeck dual-target build

## Prerequisites

**JavaScript workspace** (run before native builds; CMake copies `js/dist/runtime` into `<build>/js/`):

- Node.js 24.16+ (`.nvmrc`)
- Dependencies once: `pnpm --dir js install`
- Produce runtime bundle and example `.vdapp` packages: `pnpm --dir js build` → writes `js/dist/runtime/runtime.js` and `js/examples/*/dist/*.vdapp`
- Typecheck: `pnpm --dir js tsc`; lint/format: `pnpm --dir js lint` / `pnpm --dir js format`

**Host (current machine)** — e.g. macOS:

- CMake 3.21+ for `cmake --preset default`
- Ninja and [raylib](https://www.raylib.com/) via Homebrew: `brew install ninja raylib`
- CURL, OpenSSL, and zlib (typical on macOS; CMake `find_package` must succeed)

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

**2. Host** (executable `out/vitadeck`; reruns `pnpm build` when JS changed, so step 1 is only needed once):

```sh
cmake --preset default && cmake --build out
```

Everyday loop: `ninja -C out run` (or `cmake --build out --target run`) rebuilds JS if it changed, builds the host, stages the examples, and launches.

**3. PSVita** (artifacts under `out-vita/`, including `vitadeck.vpk` when packaging is enabled):

```sh
docker compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"
```

The `upload_vita`, `upload_vpk`, and `run` targets send the build to a Vita over FTP. They need the Vita's IP in `PSVITAIP` at configure time (Compose passes it into the container), e.g. `PSVITAIP=192.168.1.177 docker compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita --target run"`. They fail with a message when it is unset.

`upload_vpk` copies the VPK to `ux0:/download/` for a manual install. `upload_vita` (and `run`) stops the running app, uploads `eboot.bin`, the runtime and the fonts, then relaunches. It also uploads `assets/emoji` (about 4,000 PNGs in one curl run), but only the first time per build directory and Vita IP and again when the images change, tracked by `out-vita/upload_emoji-<ip>.stamp`; delete that stamp to force a re-upload, e.g. after removing the app from the Vita.

Without the device, run the VPK in the Vita3K emulator with `scripts/vita3k.sh` (skill `vitadeck-emulator`).

## Workflows

### Canonical agent checks

From the repo root, prefer the tiered verification script:

- `scripts/agent-check.sh fast`: JS typecheck, JS lint, and C formatting check.
- `scripts/agent-check.sh host`: JS build, host configure/build, and non-smoke CTest.
- `scripts/agent-check.sh smoke`: host build plus visual smoke CTest.
- `scripts/agent-check.sh full`: fast, host, smoke, and Vita Docker build.

### After JS, TSX, or deck-app changes

1. Rebuild host: `cmake --build out` reruns `pnpm build` (run `pnpm --dir js tsc` when checking types).
2. Rebuild Vita: `pnpm --dir js build` first, then `docker compose run --rm vitasdk bash -lc "cmake --build out-vita"` (or full configure in the container if CMake changed).

### After C or CMake changes

- C sources under `src/` and `tests/` use **clang-format** (`.clang-format` at repo root; editor: **clangd** with format-on-save). Check: `./scripts/clang-format.sh check`; apply: `./scripts/clang-format.sh format`. Run `cmake --preset default` so `out/compile_commands.json` exists for clangd.

1. Rebuild host: `cmake --build out` (or full configure line if `CMakeLists.txt` changed).
2. Rebuild Vita: ensure `js/dist/` is current with **`pnpm --dir js build`**, then `docker compose run --rm vitasdk bash -lc "cmake --build out-vita"` (or full configure inside the container if CMake changed).

### Clean or switch generators

- Host: remove `out/`, then configure + build.
- Vita: remove `out-vita/` inside the same `docker compose run ... bash -lc "..."` pattern, then configure and build again.

### Verify both targets

Run **JS build**, then the host quick start, then the Vita quick start. Fix compile errors on each target separately — `CMakeLists.txt` branches on `VITASDK` for Vita vs raylib for host.

## Notes

- Configure downloads the pinned DejaVu Sans, Droid Sans Fallback Full, Noto Sans KR and Twemoji releases once into the gitignored `assets/fonts/` and `assets/emoji/` (`cmake/fonts.cmake`, `cmake/twemoji.cmake`, SHA-256 checked by `cmake/fetch_asset.cmake`); host and VPK builds both ship them. Offline it warns and builds with fallbacks: raylib's built-in font for the Runtime Default Font, missing-glyph boxes for CJK and Korean, a fallback glyph for emoji. The next configure retries.
- In the container, `VITASDK` is set by the image; the project picks the Vita toolchain when that env var is present.
- Host builds use `out/`; Vita builds use `out-vita/` on the **host** filesystem (bind-mounted from the container).
- Host only: `cmake --build out --target stage_examples` copies every built `js/examples/*/dist/*.vdapp` into `out/vitadeck-data/installed-deck-apps`; `cmake --build out --target run` builds, stages, and launches `out/vitadeck`.
- Host only: the `js` target (a dependency of `vitadeck`, `stage_examples`, `run`, and the smoke harnesses) runs `pnpm build` when the JS inputs globbed in `CMakeLists.txt` are newer than `js/dist/runtime/runtime.js`. Cleaning a host build removes that file from the source tree, so the next host build reruns pnpm and a Vita build needs `pnpm --dir js build` first. The Vita build never runs pnpm.
- `out/js` and `out/assets` are symlinks into the source tree, created at configure time, so the host reads the current runtime and assets without a copy step.
- The **`vitadeck`** CLI is a compiled **`js/packages/sdk/dist/cli.mjs`** (plus **`dist/templates/`**); **`pnpm --dir js install`** runs the workspace **`prepare`** script, which builds **`@vitadeck/sdk`**.
