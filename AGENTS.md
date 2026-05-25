# Agents

## Cursor Cloud specific instructions

### Overview

VitaDeck is a C + TypeScript application that runs React-based "Deck Apps" on PlayStation Vita (and desktop for development). It uses raylib for rendering and QuickJS for JavaScript execution.

### Running services

Build commands are documented in `.cursor/skills/vitadeck-build/SKILL.md` and `.cursor/rules/macos-build.mdc`. Key differences on Linux Cloud VMs:

- **JS workspace**: `pnpm --dir js install && pnpm --dir js build` (must run before native builds)
- **Native build on Linux**: requires `CC=gcc CXX=g++ cmake -S . -B out -DCMAKE_EXE_LINKER_FLAGS="-Wl,--start-group"` then `cmake --build out`. The `--start-group` linker flag is needed on Linux/GCC because the default library ordering causes unresolved `libm` symbols from QuickJS. This is not needed on macOS.
- **Run the app**: `cd out && ./vitadeck` (run from `out/` directory so it finds `js/runtime.js` copied there by CMake)
- **Upload Deck Apps**: while the app is running with the Upload screen active (Shell > Upload), use `curl -X POST http://localhost:8787/upload -F archive=@path/to/app.vdapp.zip`

### Lint / typecheck

- Lint: `pnpm --dir js lint`
- Typecheck: `pnpm --dir js tsc`
- Format check: `pnpm --dir js format:check` (note: `oxfmt` reports issues on generated `dist/` files — this is expected)

### System dependencies (Linux)

The update script handles JS dependencies. These system packages must be pre-installed on the VM (handled during initial setup, not the update script):

- raylib 5.5 (built from source at `/usr/local/lib`)
- `libcurl4-openssl-dev`, `libssl-dev`, `zlib1g-dev`
- `libgl-dev`, `libegl-dev`, `libx11-dev` and related X11/Wayland dev packages
- `gcc`, `g++` (clang on this VM has a broken `lstdc++` link)

### Git submodules

QuickJS is a git submodule at `vendor/quickjs`. Run `git submodule update --init --recursive` if the directory is empty.
