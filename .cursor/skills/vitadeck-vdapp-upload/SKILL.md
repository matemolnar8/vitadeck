---
name: vitadeck-vdapp-upload
description: Uploads a Deck App Runtime Upload Archive (.vdapp.zip) to a Vita running VitaDeck over LAN. Use when the user asks to upload, publish, or deploy a vdapp to Vita/PSVita, push a deck app to hardware, or curl a .vdapp.zip to the Runtime Upload listener.
---

# VitaDeck vdapp upload

## Prerequisite

VitaDeck must be running on the Vita with **Runtime Upload** active (Shell → **Upload** → confirm). The listener serves `POST /upload` on port **8787** (falls forward if busy).

## Quick start

From any directory, upload a built archive:

```sh
/path/to/vitadeck/.cursor/skills/vitadeck-vdapp-upload/scripts/upload-vdapp.sh /path/to/myapp.vdapp.zip
```

Success looks like: `{"ok":true,"packageName":"myapp.vdapp","version":"0.1.0"}`

## Build the archive (if needed)

Inside a Deck App project (this repo or elsewhere):

```sh
vitadeck build
```

Produces `<outDir>/<name>.vdapp/` and `<outDir>/<name>.vdapp.zip` (e.g. `dist/chat.vdapp.zip`). Use the **`.zip`**, not the unpacked folder.

## Vita IP

Override explicitly:

```sh
upload-vdapp.sh --ip 192.168.1.50 myapp.vdapp.zip
# or
PSVITAIP=192.168.1.50 upload-vdapp.sh myapp.vdapp.zip
```

Default resolution (first hit wins):

1. `--ip` / `PSVITAIP`
2. `out-vita/CMakeCache.txt` → `PSVITAIP:STRING=…`
3. `out/CMakeCache.txt` → `PSVITAIP:STRING=…`
4. `CMakeLists.txt` → `set(PSVITAIP "…")` (repo default)

## Manual curl

```sh
curl -X POST "http://${PSVITAIP:-192.168.1.177}:8787/upload" \
  -F archive=@/path/to/myapp.vdapp.zip
```

## Troubleshooting

- **Connection refused** — Upload screen not open on Vita, or wrong IP.
- **409** — Another upload in progress; wait and retry.
- **422** — Bad archive layout; zip must contain exactly one top-level `*.vdapp` directory.
- After upload, pick the package on **Shell Home Screen** (Enter) if it did not become active automatically.
