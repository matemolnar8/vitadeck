---
name: vitadeck-manual-testing
description: Guides manual GUI testing of VitaDeck Deck Apps, Shell navigation, Runtime Upload, Host Control linking, and walkthrough recording. Use when testing VitaDeck visually, driving the raylib window (computerUse, xdotool, Xvfb, headless or over SSH), validating Deck App rendering, or producing screenshots/videos for UI changes.
metadata:
  internal: true
---

# VitaDeck manual testing

## Quick start

1. Build with `cmake --preset default && cmake --build out`; the host build reruns `pnpm build` when JS changed.
2. Stage Deck Apps for local runs when `out/vitadeck-data` is missing or stale:
   - `cmake --build out --target stage_examples` copies every built example into `out/vitadeck-data/installed-deck-apps/`.
   - Pick the one to run: `printf 'chat.vdapp\n' > out/vitadeck-data/active-package.txt`. Without this file the Shell opens with no Active Deck App.
3. Without a desktop you can see (SSH, a locked session, ubuntu-pc), start a private display first: `Xvfb :77 -screen 0 1280x800x24 -nolisten tcp &`, then `export DISPLAY=:77` and `unset WAYLAND_DISPLAY`.
4. Run from `out/`, not the repo root, under the machine-wide GUI lock so it does not overlap a benchmark, in the background with its log in a file: `../scripts/gui-run.sh ./vitadeck > /tmp/vitadeck.log 2>&1 &`.
5. Drive the window with `computerUse` when you have it, otherwise with `../scripts/gui-drive.sh` (`key F1`, `click <x> <y>`, `shot <file.png>`; `--help` lists the rest). Record only the final successful walkthrough.

## Navigation map

- **Start / F1** toggles between the running **Deck App** and the **VitaDeck Shell**.
- In the **VitaDeck Shell**, **Enter** confirms and **Backspace** goes back.
- The **Shell Home Screen** lists installed **Deck App Packages**, then **Upload and link** (the LAN Listener, on from startup; Enter turns it off or on), then **Host Control** (the linked computers; Enter on a computer forgets it).
- Hiding the Shell should reveal the **Active Deck App**.

## Reliable local Deck App verification

Use direct staging for deterministic UI tests. Runtime Upload is useful only when the upload flow itself is under test.

Checklist:

- Confirm `js/examples/<name>/dist/<name>.vdapp/manifest.json` matches the expected package.
- Confirm package assets exist, e.g. `js/examples/chat/dist/chat.vdapp/fonts/*.ttf`.
- Copy the built `.vdapp` directory into `out/vitadeck-data/installed-deck-apps/<name>.vdapp`.
- Set `out/vitadeck-data/active-package.txt` to that package name.
- Restart `out/vitadeck` after changing staged package contents.
- Read the log file for startup evidence such as loaded package fonts.

## Runtime Upload testing

Only use this when testing upload behavior:

1. VitaDeck listens from startup; open the **Shell** with F1 to read the URL under **Upload and link**.
2. Prefer the known local endpoint:
   - `curl -X POST http://localhost:8787/upload -F archive=@js/examples/chat/dist/chat.vdapp.zip`
3. Verify upload actually replaced the package on disk before judging the UI:
   - `out/vitadeck-data/installed-deck-apps/<package>.vdapp/manifest.json`
4. If curl resets or hangs, inspect the app log. Do not assume the package changed.

## Host Control testing

1. Start a companion from the repo: `pnpm --dir js host-companion start`. Use `--config /tmp/<name>.json` to keep tests out of `~/.vitadeck`, and `--port 8799` for a second companion on the same computer. On another computer, use the published one instead of a checkout: `npm install -g @vitadeck/host-companion`, then `vitadeck-host-companion start`.
2. Run `pnpm --dir js host-companion link <address> --config <same config> [--port <same port>]` with the address the Shell shows under **Upload and link** (or **Shell → Host Control**). That screen lists the linked computers.
3. Run `host-control.vdapp` for live gauges and Echo. The companion logs one line per request, which helps to tell a UI bug from a missing call.
4. For a custom plugin, build its project and pass `--plugin <project>/dist` to `start`. Or `install` the `.vdplugin.zip` with the same `--config`.
5. Reverse states: stop the companion (the app shows `unreachable`, or `timeout` when a firewall drops the connection, as on Windows), `remove <id>` (`unknown_plugin`), and forget the computer in the Shell (`not_linked`).

Links persist in `out/vitadeck-data/host-control-links.json`; delete it to start unlinked.

## Walkthrough recording pattern

1. Complete all setup before recording. To start a Deck App from a clean state, delete `out/vitadeck-data/deck-app-data/<package>/storage.json`.
2. Start recording the window rect from `../scripts/gui-drive.sh rect` (`x,y,width,height`):
   - Linux: `ffmpeg -loglevel error -y -f x11grab -draw_mouse 0 -framerate 30 -video_size <width>x<height> -i "$DISPLAY+<x>,<y>" -c:v libx264 -pix_fmt yuv420p take.mp4 &`; stop it with `kill -INT`.
   - macOS: `screencapture -x -v -V <seconds> -R<rect> take.mov`, then `avconvert --preset PresetHighestQuality --source take.mov --output take.mp4`.
   - Sound on Linux: record PipeWire's sink monitor with `pw-record --target <sink id from wpctl status> -P '{ stream.capture.sink=true }' take.wav`.
3. Perform only the visual verification, with `computerUse` or `../scripts/gui-drive.sh`.
4. Save the recording only if the walkthrough succeeds; otherwise discard it, fix the issue, and retry.
5. For font or text changes, verify both:
   - startup logs show the expected font/package loaded;
   - the rendered UI visibly demonstrates the requested typography.

## Common pitfalls

- Running from the repo root makes `vitadeck` miss `js/runtime.js`; always run from `out/`.
- `pnpm --dir js build` rebuilds example `.vdapp` packages; it does not automatically update `out/vitadeck-data`.
- A visible old UI usually means the installed package was not replaced or the app was not restarted.
- The listener falls forward to 8788-8796 when 8787 is busy; read the port from the URL under **Upload and link** in the Shell before curling or linking.
- Do not keep failed walkthrough recordings as artifacts.
