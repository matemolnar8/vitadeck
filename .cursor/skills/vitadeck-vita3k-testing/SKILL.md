---
name: vitadeck-vita3k-testing
description: Builds out-vita3k/vitadeck.vpk with HAVE_VITA3K_SUPPORT and runs it in Vita3K for emulator smoke testing with computerUse. Use when validating Vita builds without hardware, testing VPK boot/rendering/shell flow, or when the user mentions Vita3K, PSVita emulator, or VPK testing.
---

# VitaDeck Vita3K testing

Vita3K smoke tests require a **Vita3K-target VPK** built with `HAVE_VITA3K_SUPPORT=1` (output: `out-vita3k/vitadeck.vpk`). Do **not** reuse `out-vita/` or `vitadeck-vpk-*` hardware CI artifacts — they crash on launch (`SCE_GXM_ERROR_ALREADY_INITIALIZED`).

**Title ID:** `PGRI00001`

## Build the VPK (required)

```sh
pnpm --dir js build
scripts/vita3k-deploy.sh build-vpk
# → out-vita3k/vitadeck.vpk
```

This uses the `vitasdk-vita3k` Docker Compose service (`Dockerfile` with `VITA3K_SUPPORT=1`), which rebuilds vitaGL/SDL/raylib for emulator GXM init.

### Cloud agents and Docker

Docker is **not pre-installed** on Cursor Cloud VMs. It can be installed, but **image builds often fail** inside cloud agents (nested overlay/`buildkit` mount errors). Prefer CI for cloud agents:

```sh
# Optional local attempt (works on dev machines; may fail on cloud VMs):
sudo apt-get update && sudo apt-get install -y docker.io docker-compose-v2
sudo dockerd >/tmp/dockerd.log 2>&1 &
sleep 2 && scripts/vita3k-deploy.sh build-vpk
```

**Cloud agent fallback** — wait for CI job `vpk-vita3k`, then:

```sh
scripts/vita3k-deploy.sh download-vpk
```

## Deploy and launch

```sh
scripts/vita3k-deploy.sh setup-vita3k   # once, for extracted AppImage
scripts/vita3k-deploy.sh install-vpk
scripts/vita3k-deploy.sh seed-deck-app

SESSION_NAME="vita3k-vitadeck"
tmux -f /exec-daemon/tmux.portal.conf new-session -d -s "$SESSION_NAME" \
  "DISPLAY=:1 \$HOME/vita3k/squashfs-root/usr/bin/Vita3K -r PGRI00001"
```

Never wrap Vita3K in `timeout`. Deck App names must end in `.vdapp`.

## computerUse workflow

Same navigation as `vitadeck-manual-testing` once VitaDeck boots:

- **Start / F1** — Deck App ↔ Shell
- **Enter** / **Backspace** — Shell navigation

Before computerUse:

1. Confirm `out-vita3k/vitadeck.vpk` exists (built locally or downloaded).
2. Set renderer to **OpenGL** on cloud VMs (`~/.config/Vita3K/config.yml`: `backend-renderer: OpenGL`).
3. Check tmux log on failure:

```sh
tmux -f /exec-daemon/tmux.portal.conf capture-pane -p -t vita3k-vitadeck:0.0 -S -80
```

## Deploy commands

| Command | Purpose |
| --- | --- |
| `build-vpk` | Docker build → `out-vita3k/vitadeck.vpk` (**primary**) |
| `download-vpk` | CI fallback: `vitadeck-vpk-vita3k-*` artifact only |
| `setup-vita3k` | Shader symlink for extracted AppImage |
| `install-vpk` | Unzip into `ux0/app/PGRI00001/` |
| `seed-deck-app` | Stage `js/dist/deck-app` → `ux0:data/vitadeck/` |
| `sync` | Fast copy `eboot.bin` + `js/runtime.js` after rebuild |

## Why two VPK outputs?

| Output | Docker service | vitaGL flags | Use for |
| --- | --- | --- | --- |
| `out-vita/vitadeck.vpk` | `vitasdk` | `HAVE_GLSL_SUPPORT=1` | Real Vita hardware |
| `out-vita3k/vitadeck.vpk` | `vitasdk-vita3k` | `+ HAVE_VITA3K_SUPPORT=1` | Vita3K emulator |

Hardware builds call `sceGxmVshInitialize`; Vita3K builds call `sceGxmInitialize` with simpler flags.

## Prerequisites (emulator)

1. **Vita3K** — [continuous release](https://github.com/Vita3K/Vita3K/releases/tag/continuous). Linux: extract AppImage with `--appimage-extract` if FUSE is missing.
2. **Firmware (recommended)** — `PSVUPDAT.PUP` + `PSP2UPDAT.PUP` via **File → Install Firmware**.
3. **JS bundle** — `pnpm --dir js build` before seeding Deck Apps.

**Linux pref-path:** `~/.local/share/Vita3K/Vita3K/`

## Limitations

- Not a hardware replacement; use `upload_vita` for FTP deploy to real Vitas.
- Runtime Upload over HTTP is indicative only on emulator.
- CI cannot run Vita3K GUI tests — agents/computerUse verify locally.

## Related skills

- `vitadeck-build` — hardware `out-vita/` builds (`vitasdk` service)
- `vitadeck-manual-testing` — host raylib testing and navigation reference
