---
name: vitadeck-vita3k-testing
description: Installs and runs out-vita/vitadeck.vpk in Vita3K for PSVita-target smoke testing. Use when validating Vita builds without hardware, testing VPK boot/rendering/shell flow, or when the user mentions Vita3K, PSVita emulator, or VPK testing.
---

# VitaDeck Vita3K testing

Vita3K is **feasible as a dev smoke-test** for the Vita target: install the built VPK, seed Deck App data under `ux0:data/vitadeck`, and verify boot, shell navigation, and rendering. It is **not** a replacement for hardware — use it alongside the host build (`vitadeck-manual-testing`) and real-Vita `upload_vita` when available.

## Feasibility summary

| Area | Vita3K | Notes |
| --- | --- | --- |
| VPK install / launch | Yes | GUI install or unzip into `ux0/app/PGRI00001` |
| raylib + vitaGL rendering | Yes, with build flags | Prefer `HAVE_VITA3K_SUPPORT=1` when building vitaGL (see below) |
| Shell + Deck App UI | Yes | Seed `ux0:data/vitadeck` like desktop `vitadeck-data` |
| JS runtime / QuickJS | Yes | Same `js/runtime.js` path as hardware |
| Incremental deploy | Yes | Copy `eboot.bin` + `js/runtime.js` after first install |
| Runtime Upload (HTTP) | Partial | Port 8787 may work locally; `sceNetCtl` IP display differs from hardware |
| FTP / `upload_vita` workflow | No | Use file copy into Vita3K `pref-path` instead |
| CI / headless automation | Poor | Needs GUI + firmware; do not wrap launches in `timeout` |

**Title ID:** `PGRI00001` (from `CMakeLists.txt`).

## Prerequisites

1. **Built Vita artifacts** — follow `vitadeck-build`: `pnpm --dir js build`, then Docker Vitasdk build → `out-vita/vitadeck.vpk`.
2. **Vita3K** — [continuous release](https://github.com/Vita3K/Vita3K/releases/tag/continuous):
   - Linux x86_64: `Vita3K-x86_64.AppImage` (extract with `--appimage-extract` if FUSE is unavailable)
   - macOS: `macos-latest.dmg` or `macos-arm64-latest.dmg`
   - Windows: `windows-latest.zip`
3. **Firmware (one-time)** — install **both** in Vita3K before running homebrew:
   - Main: `PSVUPDAT.PUP` (3.65 or 3.74 from PlayStation update pages)
   - Fonts: `PSP2UPDAT.PUP` (font package; see [Vita3K quickstart](https://vita3k.org/quickstart.html))
   - GUI: **File → Install Firmware** (install each file once)
   - CLI (main firmware only): `Vita3K --firmware /path/to/PSVUPDAT.PUP`

**Linux pref-path:** `~/.local/share/Vita3K/Vita3K/`

## Quick start

```sh
# 1. Build (see vitadeck-build skill)
pnpm --dir js build
docker-compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"

# 2. Install VPK into emulator (pick one)
#    A) GUI: File → Install .zip, .vpk → select out-vita/vitadeck.vpk
#    B) CLI unzip (no GUI dialog):
scripts/vita3k-deploy.sh install-vpk

# 3. Seed an active Deck App (required — VPK does not bundle deck apps)
scripts/vita3k-deploy.sh seed-deck-app chat

# 4. Launch (keep running — do NOT use timeout on the GUI)
Vita3K -r PGRI00001
```

Run Vita3K in a dedicated tmux session (e.g. `vita3k`) when testing on Linux with a display. Prefer `computerUse` for visual verification, same as `vitadeck-manual-testing`.

## Workflows

### First-time emulator setup

1. Download and launch Vita3K once; complete language / pref-path wizard if shown.
2. Install both firmware `.pup` files (main + font package).
3. In **Configuration → Settings → Core**, leave **Modules Mode** on **Automatic** unless a module crash is suspected.
4. Optional renderer: **OpenGL** is the safer default; try **Vulkan** if rendering glitches.

### Install or update the VPK

**GUI (recommended first time):** drag `out-vita/vitadeck.vpk` onto the Vita3K window, or **File → Install .zip, .vpk**.

**CLI unzip** (repeatable, no install dialog):

```sh
scripts/vita3k-deploy.sh install-vpk
```

Positional `Vita3K /path/to/file.vpk` often opens the emulator without installing ([issue #3115](https://github.com/Vita3K/Vita3K/issues/3115)); prefer GUI or unzip.

### Fast iteration after C or JS changes

Skip full VPK reinstall — copy built artifacts only:

```sh
scripts/vita3k-deploy.sh sync
Vita3K -r PGRI00001
```

This mirrors the `upload_vita` CMake target but writes into the Vita3K `pref-path`.

### Seed Deck App data

Vita stores packages under `ux0:data/vitadeck/` (see `src/core/package_library.c`). On Vita3K:

```sh
scripts/vita3k-deploy.sh seed-deck-app chat
# or manually:
V3K="$HOME/.local/share/Vita3K/Vita3K"
mkdir -p "$V3K/ux0/data/vitadeck/installed-deck-apps"
cp -R js/dist/deck-app "$V3K/ux0/data/vitadeck/installed-deck-apps/chat.vdapp"
printf 'chat.vdapp\n' > "$V3K/ux0/data/vitadeck/active-package.txt"
```

Restart VitaDeck after changing seeded packages.

### What to verify

Use the same navigation map as `vitadeck-manual-testing`:

- **Start / F1** — Deck App ↔ Shell
- **Shell** — list installed packages, **Upload** entry
- Deck App renders with expected fonts and layout
- Check Vita3K log window for `TraceLog` / startup errors

### Runtime Upload on emulator

Only when explicitly testing upload behavior:

1. Open Shell → Upload.
2. From the host: `curl -X POST http://localhost:8787/upload -F archive=@js/examples/chat/dist/chat.vdapp.zip`
3. Confirm files under `$V3K/ux0/data/vitadeck/installed-deck-apps/`.

Network behavior can differ from a real Vita; treat pass/fail as indicative, not authoritative.

## Vita3K-compatible builds (important)

The repo Docker image builds vitaGL with `HAVE_GLSL_SUPPORT=1` for **real hardware**. vitaGL also documents `HAVE_VITA3K_SUPPORT=1` for emulator compatibility ([vitaGL README](https://github.com/Rinnegatamante/vitaGL)).

If VitaDeck **crashes or renders black** on Vita3K but works on hardware:

1. Rebuild vitaGL in the Vitasdk image with `HAVE_VITA3K_SUPPORT=1` (and vitaShaRK adjusted per upstream docs).
2. Rebuild `out-vita` from scratch.
3. Reinstall the VPK.

Until a dedicated `out-vita3k` target exists, document any local Dockerfile tweak in the PR that needs emulator testing.

## Platform notes

### Linux / Cloud VM

- AppImage may fail without FUSE: `./Vita3K-x86_64.AppImage --appimage-extract`, then run `./squashfs-root/usr/bin/Vita3K`.
- Requires a display (`DISPLAY` set, X11 socket present).
- **Do not** pipe Vita3K through `timeout` — it launches a long-lived GUI; use tmux instead.

### macOS

- Install from the `.dmg`; pref-path defaults to `~/Library/Application Support/Vita3K/`.
- Adjust `VITA3K_PREF` in `scripts/vita3k-deploy.sh` or export it before running.

## Limitations (when to use hardware instead)

- Shader / vitaGL paths differ; emulator-specific build flags may be required.
- Performance, touch, and motion are approximated.
- `upload_vita` / FTP / `make run` targets target a real Vita IP — not Vita3K.
- Automated CI smoke tests on Vita3K are impractical (firmware, GUI, GPU).

## Related skills

- `vitadeck-build` — produce `out-vita/vitadeck.vpk`
- `vitadeck-manual-testing` — host raylib testing and navigation reference
