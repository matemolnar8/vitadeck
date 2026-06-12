---
name: vitadeck-vita3k-testing
description: Installs and runs out-vita/vitadeck.vpk in Vita3K for PSVita-target smoke testing with computerUse. Use when validating Vita builds without hardware, testing VPK boot/rendering/shell flow, or when the user mentions Vita3K, PSVita emulator, or VPK testing.
---

# VitaDeck Vita3K testing

Vita3K can validate **VPK install, app discovery, and boot attempts** without hardware. As of hands-on testing (2026-06), **CI-built VPKs crash on launch** until vitaGL is rebuilt with `HAVE_VITA3K_SUPPORT=1`. Treat Vita3K as a complement to host testing (`vitadeck-manual-testing`) and real-Vita `upload_vita`.

**Title ID:** `PGRI00001`

## Feasibility (verified)

| Area | Status | Notes |
| --- | --- | --- |
| VPK install (unzip) | Works | `scripts/vita3k-deploy.sh install-vpk` |
| App appears in Vita3K library | Works | PGRI00001 visible after install |
| Boot with default CI VPK | **Fails** | SIGSEGV after `SCE_GXM_ERROR_ALREADY_INITIALIZED` |
| Deck App UI / Shell | Not reached | Blocked by GXM crash with current build |
| GH Actions VPK download | Works | `scripts/vita3k-deploy.sh download-vpk` |
| computerUse GUI testing | Works | For emulator setup + launch; read tmux logs for crashes |
| Runtime Upload | Untested | Blocked until app boots |
| Real-hardware parity | No | Requires Vita3K-specific vitaGL build |

## Prerequisites

1. **VPK** — local Docker build or CI artifact (see below).
2. **JS bundle** — `pnpm --dir js build` (needed to seed Deck Apps).
3. **Vita3K** — [continuous release](https://github.com/Vita3K/Vita3K/releases/tag/continuous). On Linux without FUSE: extract AppImage with `--appimage-extract`, then run `squashfs-root/usr/bin/Vita3K`.
4. **Firmware (recommended)** — install both via **File → Install Firmware**:
   - `PSVUPDAT.PUP` (main firmware)
   - `PSP2UPDAT.PUP` (font package)
   - CLI for main only: `Vita3K --firmware /path/to/PSVUPDAT.PUP`

**Linux pref-path:** `~/.local/share/Vita3K/Vita3K/`

## Quick start

```sh
pnpm --dir js build

# VPK: build locally OR download from CI
scripts/vita3k-deploy.sh download-vpk
# scripts/vita3k-deploy.sh download-vpk 27437425077   # optional: specific run id

scripts/vita3k-deploy.sh setup-vita3k   # once, for extracted AppImage
scripts/vita3k-deploy.sh install-vpk
scripts/vita3k-deploy.sh seed-deck-app  # → chat.vdapp

# Launch in tmux (never wrap in timeout)
SESSION_NAME="vita3k-vitadeck"
tmux -f /exec-daemon/tmux.portal.conf new-session -d -s "$SESSION_NAME" \
  "DISPLAY=:1 \$HOME/vita3k/squashfs-root/usr/bin/Vita3K -r PGRI00001"
```

Deck App package names **must** end in `.vdapp` (see `package_library.c`).

## computerUse workflow

Use this after CLI setup above. Same pattern as `vitadeck-manual-testing`, but the window is Vita3K (Qt), not raylib directly.

### Before calling computerUse

1. Confirm VPK + deck app are deployed (`install-vpk`, `seed-deck-app`).
2. Start Vita3K in tmux session `vita3k-vitadeck` (see quick start).
3. On cloud VMs, set renderer to **OpenGL** — Vulkan fails with `ErrorIncompatibleDriver`:
   - GUI: **Configuration → Settings → GPU → Backend Renderer → OpenGL**
   - Or edit `~/.config/Vita3K/config.yml`: `backend-renderer: OpenGL`
4. Run `scripts/vita3k-deploy.sh setup-vita3k` if shader load errors appear.

### computerUse prompt checklist

Ask computerUse to:

1. Focus the Vita3K window on `DISPLAY=:1`.
2. Confirm **VitaDeck** (PGRI00001) appears in the app list.
3. Launch VitaDeck (double-click or select + Start).
4. If boot succeeds: verify chat Deck App UI, then **F1 / Start** for Shell toggle, **Enter** / **Backspace** for Shell navigation (same map as `vitadeck-manual-testing`).
5. If boot fails: capture the on-screen error and check tmux log:

```sh
tmux -f /exec-daemon/tmux.portal.conf capture-pane -p -t vita3k-vitadeck:0.0 -S -80
```

### Known crash (current CI VPK)

Logs show raylib modules loading, then:

```
sceGxmCreateContext returned SCE_GXM_ERROR_ALREADY_INITIALIZED (0x805B0001)
Unhandled SIGSEGV
```

This matches a **hardware-targeted vitaGL build** running under Vita3K. Fix: rebuild vitaGL with `HAVE_VITA3K_SUPPORT=1` in the Vitasdk Docker image, produce a new VPK, reinstall, and retest. Until then, computerUse can verify install/library/launch path but not in-app UI.

### Recording

Record only a successful end-to-end walkthrough. Discard failed recordings.

## Get VPK without local Docker

```sh
gh run list --workflow="Vita (VPK)" --limit 5
scripts/vita3k-deploy.sh download-vpk
```

Manual fallback:

```sh
RUN_ID=27437425077
ARTIFACT=$(gh api repos/matemolnar8/vitadeck/actions/runs/$RUN_ID/artifacts \
  --jq '.artifacts[] | select(.name | startswith("vitadeck-vpk")) | .name')
mkdir -p out-vita
gh run download "$RUN_ID" -n "$ARTIFACT" -D out-vita
```

There are no GitHub **releases**; use Actions artifacts only.

## Deploy commands

| Command | Purpose |
| --- | --- |
| `download-vpk` | Fetch latest successful CI VPK into `out-vita/` |
| `setup-vita3k` | Symlink `shaders-builtin` next to extracted binary |
| `install-vpk` | Unzip VPK → `ux0/app/PGRI00001/` |
| `seed-deck-app` | Copy `js/dist/deck-app` → `ux0/data/vitadeck/...` |
| `sync` | Fast copy of `eboot.bin` + `js/runtime.js` (needs local `out-vita/eboot.bin`) |

Positional `Vita3K file.vpk` often opens the GUI without installing ([issue #3115](https://github.com/Vita3K/Vita3K/issues/3115)); prefer unzip.

## Seed Deck App data

VPK does not bundle Deck Apps. Package data lives at `ux0:data/vitadeck/`:

```sh
scripts/vita3k-deploy.sh seed-deck-app chat   # creates chat.vdapp
```

Restart VitaDeck after changing seeded packages.

## Vita3K-compatible builds (required for boot)

The Docker image builds vitaGL with `HAVE_GLSL_SUPPORT=1` for **real hardware**. For emulator boot, vitaGL needs `HAVE_VITA3K_SUPPORT=1` ([vitaGL README](https://github.com/Rinnegatamante/vitaGL)). Consider a dedicated Vitasdk image or `out-vita3k` target so hardware builds stay unchanged.

After a Vita3K-target build: `install-vpk` again and relaunch with computerUse.

## Platform notes

### Linux / Cloud VM

- Extract AppImage if FUSE is missing; run `setup-vita3k` once.
- Requires `DISPLAY` and an X11 socket.
- **Do not** pipe Vita3K through `timeout` — use tmux.
- Software rendering (llvmpipe) works for emulator UI; app boot still needs Vita3K-target binaries.

### macOS

- Pref-path: `~/Library/Application Support/Vita3K/Vita3K/`
- Set `VITA3K_PREF` when running deploy script from a non-default location.

## When to use hardware instead

- Any feature validation beyond “does the VPK install and attempt boot?”
- Runtime Upload, FTP/`upload_vita`, performance, touch
- Authoritative rendering or shader behaviour

## Related skills

- `vitadeck-build` — produce `out-vita/vitadeck.vpk`
- `vitadeck-manual-testing` — host raylib testing and navigation reference
