---
name: vitadeck-emulator
description: Runs the VitaDeck Vita build (VPK) in the Vita3K emulator on macOS, Windows or Linux, sets the emulator up on a new machine, and drives it. Use when the user mentions Vita3K, the emulator, testing the Vita build without the device, libshacccg.suprx, or running the VPK on a computer.
metadata:
  internal: true
---

# VitaDeck in Vita3K

`scripts/vita3k.sh` runs the Vita build without the device. It builds JS and the VPK (Docker), installs the VPK and the example Deck Apps into [Vita3K](https://vita3k.org), and launches it:

```sh
scripts/vita3k.sh          # build, then launch
scripts/vita3k.sh launch   # launch the existing out-vita/vitadeck.vpk
```

Emulator config and storage live in `~/.vitadeck/vita3k` (`VITADECK_VITA3K_HOME`), shared by every worktree. Launching waits for the machine-wide GUI lock (`scripts/gui-run.sh`) that `scripts/bench.sh` and `cmake --build out --target run` also take, so one VitaDeck window runs at a time; copying the emulator home doesn't sidestep it. `fs/` in there is the emulated Vita: `ux0/data/vitadeck` is the VitaDeck Data Root, `ur0/data` holds the shader compiler. Run `scripts/vita3k.sh help` for the environment variables.

## Set up a new machine

1. Install the Vita build prerequisites: Node.js 24.16+ (`.nvmrc`), pnpm, Docker (see `vitadeck-build`). `scripts/vita3k.sh` uses `docker compose`.
2. Install Vita3K:
   - macOS: download the dmg from [vita3k.org](https://vita3k.org) for your CPU (Intel or Apple silicon) and drag `Vita3K.app` to Applications.
   - Windows: `winget install Vita3K.Vita3K`, then run the script from Git Bash. If Vita3K isn't on PATH, set `VITA3K` to `Vita3K.exe`.
   - Linux: download the AppImage, `chmod +x` it, and set `VITA3K` to it.
3. Install `libshacccg.suprx`. vitaGL compiles its shaders at runtime with Sony's shader compiler, which can't be redistributed, so it isn't in the repo and the script won't launch without it. Use one of:
   - Copy it from a machine that already runs VitaDeck in Vita3K: `scp <machine>:.vitadeck/vita3k/fs/ur0/data/libshacccg.suprx /tmp/ && scripts/vita3k.sh shacccg /tmp/libshacccg.suprx`.
   - Download it from the Vita: run [ShaRKBR33D](https://github.com/Rinnegatamante/ShaRKBR33D) on the Vita once (it extracts the decrypted module to `ur0:data/libshacccg.suprx`), start VitaShell's FTP server, then `PSVITAIP=<ip> scripts/vita3k.sh shacccg`.
4. Optional: install the firmware. VitaDeck runs without it; it only provides the CA list that `fetch` reads (fetch still fails, see below). `Vita3K --firmware PSVUPDAT.PUP` installs into Vita3K's default storage and ignores our config, so move `os0` and `vs0` from there into `~/.vitadeck/vita3k/fs/` afterwards. On macOS that is `~/Library/Application Support/Vita3K/Vita3K/fs`.
5. `scripts/vita3k.sh`. The first launch installs the VPK; the Demo Deck App should appear.

## Controls

| Vita | Vita3K keyboard |
|---|---|
| D-pad | Arrow keys |
| Start (opens the VitaDeck Shell) | Enter |
| Cross | X |
| Circle | C |
| Square / Triangle | Z / V |
| L / R | Q / E |
| Select | Right Shift |

Start is Enter here, not F1 as in the host build.

## Driving it as an agent

- Click the Vita3K window once before sending keys. Keys sent while it is merely frontmost are dropped.
- Hold each key for about 60 ms. Longer holds trigger VitaDeck's D-pad repeat and overshoot the focus.
- The VitaDeck log is in the terminal as `export_sceClibVprintf` lines. An access violation at `0x80` logged at startup is harmless.
- Runtime Upload works: VitaDeck listens from startup, so `vitadeck dev http://127.0.0.1:8787/` or `curl -X POST http://127.0.0.1:8787/upload -F archive=@<app>.vdapp.zip` from the same machine.

## Known emulator gaps

These are Vita3K bugs, not VitaDeck bugs. Check them on the device.

- HTTPS `fetch` fails with a certificate name mismatch. Vita3K's `sceNetInetPton` returns an error instead of 0 for host names, so curl skips SNI. Without the firmware it fails earlier, on the missing `vs0:` CA list.
- The Vita keyboard dialog (Text Input) opens but takes no input.
