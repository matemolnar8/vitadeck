#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
HOME_DIR="${VITADECK_VITA3K_HOME:-$HOME/.vitadeck/vita3k}"
FS_DIR="$HOME_DIR/fs"
CONFIG="$HOME_DIR/config.yml"
SHACCCG="$FS_DIR/ur0/data/libshacccg.suprx"
VPK="$ROOT_DIR/out-vita/vitadeck.vpk"
# shellcheck source=gui-run.sh
source "$ROOT_DIR/scripts/gui-run.sh"

usage() {
  cat <<'EOF'
usage: scripts/vita3k.sh [run|launch|shacccg [file]]

run              Build JS and the VPK (Docker), then launch (default).
launch           Install out-vita/vitadeck.vpk and the example Deck Apps into Vita3K and run it.

run and launch wait for the machine-wide GUI lock (scripts/gui-run.sh) before touching the emulator.
shacccg [file]   Copy libshacccg.suprx into the emulator; without a file, download it from the Vita's FTP server at $PSVITAIP.

Environment:
  VITA3K                 Path to the Vita3K executable (default: found on PATH or in Applications)
  VITADECK_VITA3K_HOME   Emulator config and storage (default: ~/.vitadeck/vita3k)
  PSVITAIP               Vita IP for `shacccg` without a file
EOF
}

step() {
  printf '\n==> %s\n' "$1"
}

# Vita3K is a native program on Windows, so hand it C:/... paths when running under Git Bash.
native_path() {
  if command -v cygpath >/dev/null 2>&1; then cygpath -m "$1"; else printf '%s\n' "$1"; fi
}

find_vita3k() {
  if [[ -n "${VITA3K:-}" ]]; then
    printf '%s\n' "$VITA3K"
    return
  fi
  local candidate
  for candidate in Vita3K vita3k Vita3K.exe; do
    if command -v "$candidate" >/dev/null 2>&1; then
      command -v "$candidate"
      return
    fi
  done
  for candidate in /Applications/Vita3K.app "$HOME/Applications/Vita3K.app"; do
    if [[ -x "$candidate/Contents/MacOS/Vita3K" ]]; then
      printf '%s\n' "$candidate/Contents/MacOS/Vita3K"
      return
    fi
  done
  cat >&2 <<'EOF'
Vita3K not found. Install it, or set VITA3K to its executable:
  macOS:   download the dmg from https://vita3k.org and drag Vita3K.app to Applications
  Windows: winget install Vita3K.Vita3K
  Linux:   download the AppImage from https://vita3k.org, chmod +x it, and set VITA3K
EOF
  exit 1
}

write_config() {
  mkdir -p "$FS_DIR"
  cat >"$CONFIG" <<EOF
pref-path: $(native_path "$FS_DIR")/
initial-setup: true
show-welcome: false
warn-missing-firmware: false
log-level: 2
EOF
}

build() {
  step "Build JS"
  pnpm --dir "$ROOT_DIR/js" build
  step "Build VPK"
  (cd "$ROOT_DIR" && docker compose run --rm -T vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita")
}

launch() {
  local vita3k
  vita3k=$(find_vita3k)
  if [[ ! -f "$VPK" ]]; then
    echo "Missing $VPK. Run scripts/vita3k.sh run to build it." >&2
    exit 1
  fi
  if [[ ! -f "$SHACCCG" ]]; then
    cat >&2 <<EOF
Missing $SHACCCG.
vitaGL compiles its shaders with Sony's libshacccg.suprx, which cannot be redistributed. Copy the one ShaRKBR33D extracted on your Vita (ur0:data/libshacccg.suprx):
  PSVITAIP=<Vita IP> scripts/vita3k.sh shacccg     (VitaShell FTP server running)
  scripts/vita3k.sh shacccg path/to/libshacccg.suprx
EOF
    exit 1
  fi
  gui_lock launch
  write_config
  step "Stage example Deck Apps"
  "$ROOT_DIR/scripts/stage-example-apps.sh" "$ROOT_DIR" "$FS_DIR/ux0/data/vitadeck"
  step "Install and run in Vita3K (Start: Enter, Cross: X, Circle: C)"
  gui_run "$vita3k" -c "$(native_path "$CONFIG")" --frontend sdl "$(native_path "$VPK")"
}

shacccg() {
  mkdir -p "$(dirname "$SHACCCG")"
  if [[ -n "${1:-}" ]]; then
    cp "$1" "$SHACCCG"
  elif [[ -n "${PSVITAIP:-}" ]]; then
    curl -fsS -o "$SHACCCG" "ftp://$PSVITAIP:1337/ur0:/data/libshacccg.suprx"
  else
    echo "Pass a file, or set PSVITAIP to download it from the Vita." >&2
    exit 1
  fi
  echo "Installed $SHACCCG"
}

case "${1:-run}" in
  run)
    "$ROOT_DIR/scripts/check-macos-display.sh"
    build
    launch
    ;;
  launch)
    "$ROOT_DIR/scripts/check-macos-display.sh"
    launch
    ;;
  shacccg)
    shacccg "${2:-}"
    ;;
  -h | --help | help)
    usage
    ;;
  *)
    usage >&2
    exit 1
    ;;
esac
