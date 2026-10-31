---
name: vitadeck-vdapp-upload
description: Uploads a Deck App Runtime Upload Archive (.vdapp.zip) to a Vita running VitaDeck over LAN. Use when the user asks to upload, publish, or deploy a vdapp to Vita/PSVita, push a deck app to hardware, curl a .vdapp.zip to the LAN Listener, or run `vitadeck dev` to rebuild and upload on every save.
---

# VitaDeck vdapp upload

## Prerequisite

VitaDeck must be running on the Vita. It listens from startup and serves `POST /upload` on port **8787** (falls forward through 8796 if busy; the Shell shows the bound URL under **Upload and link**, pass its port with `--port`). Turning **Upload and link** off in the Shell stops it until it is turned back on or VitaDeck restarts.

## Preferred: `vitadeck dev`

From the Deck App root, pass the URL shown under **Upload and link** in the Shell:

```sh
npx @vitadeck/sdk dev http://192.168.1.50:8787/   # or `vitadeck dev` inside this repo
```

It builds with `--dev`, uploads, then rebuilds and re-uploads on every save, printing one line per upload (`installed myapp.vdapp 0.1.0` or `422 invalid_package: …`). Activate the app once in the Shell; later uploads replace it and restart it. Ctrl-C stops.

## Fallback: upload a built archive

From any directory, upload a built archive:

```sh
/path/to/vitadeck/.agents/skills/vitadeck-vdapp-upload/scripts/upload-vdapp.sh /path/to/myapp.vdapp.zip
```

Success looks like: `{"ok":true,"packageName":"myapp.vdapp","version":"0.1.0"}`

## Build the archive (if needed)

Inside a Deck App project (this repo or elsewhere):

```sh
vitadeck build
```

Produces `<outDir>/<name>.vdapp/` and `<outDir>/<name>.vdapp.zip` (e.g. `dist/chat.vdapp.zip`). Use the **`.zip`**, not the unpacked folder. A project with a Host Control Plugin also writes `<name>.vdplugin.zip`; that one is never uploaded to the Vita, it is installed on the computer with `vitadeck-host-companion install` (see [vitadeck-host-control](../vitadeck-host-control/SKILL.md)).

## Vita IP

Pass `--ip`, or set `PSVITAIP` (the same variable the Vita `upload_vita` / `upload_vpk` CMake targets read):

```sh
upload-vdapp.sh --ip 192.168.1.50 myapp.vdapp.zip
# or
PSVITAIP=192.168.1.50 upload-vdapp.sh myapp.vdapp.zip
```

## Manual curl

```sh
curl -X POST "http://${PSVITAIP}:8787/upload" \
  -F archive=@/path/to/myapp.vdapp.zip
```

## Troubleshooting

- **Connection refused** — wrong IP, VitaDeck not running, or **Upload and link** turned off in the Shell.
- **409** — Another upload in progress; wait and retry (`dev` retries once, then on the next save).
- **422** — Bad archive layout; zip must contain exactly one top-level `*.vdapp` directory.
- After upload, pick the package on **Shell Home Screen** (Enter) if it did not become active automatically.
