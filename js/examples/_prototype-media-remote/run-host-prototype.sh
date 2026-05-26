#!/usr/bin/env bash
set -euo pipefail
ROOT="$(cd "$(dirname "$0")/../../.." && pwd)"
APP_SLUG="prototype-media-remote"
DATA_DIR="${VITADECK_DATA:-$ROOT/vitadeck-data}"
HOST_URL="${HOST_CONTROL_URL:-http://127.0.0.1:8797}"

echo "Building SDK, companion, and prototype Deck App..."
pnpm --dir "$ROOT/js" --filter @vitadeck/sdk build
pnpm --dir "$ROOT/js" --filter @vitadeck/host-control-companion build
pnpm --dir "$ROOT/js/examples/_prototype-media-remote" build

mkdir -p "$DATA_DIR/installed-deck-apps"
rm -rf "$DATA_DIR/installed-deck-apps/${APP_SLUG}.vdapp"
cp -R "$ROOT/js/examples/_prototype-media-remote/dist/${APP_SLUG}.vdapp" "$DATA_DIR/installed-deck-apps/"

mkdir -p "$DATA_DIR"
printf '%s' "$HOST_URL" > "$DATA_DIR/host-control-url.txt"
echo "Host Control URL: $HOST_URL"

if ! pgrep -f "host-control-companion" >/dev/null 2>&1; then
  echo "Starting Host Control Companion in background..."
  pnpm --dir "$ROOT/js" --filter @vitadeck/host-control-companion start &
  sleep 1
fi

if [[ ! -x "$ROOT/out/vitadeck" ]]; then
  echo "Building native vitadeck (first run)..."
  CC=gcc CXX=g++ cmake -S "$ROOT" -B "$ROOT/out" -DCMAKE_EXE_LINKER_FLAGS="-Wl,--start-group"
  cmake --build "$ROOT/out"
fi

echo "Launching vitadeck with installed prototype..."
cd "$ROOT/out" && exec ./vitadeck
