#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${VITADECK_OUT:-$ROOT/out}"
DISPLAY="${DISPLAY:-:99}"
VITA_URL="${VITA_URL:-http://127.0.0.1:8787}"

if ! pgrep -f "Xvfb $DISPLAY" >/dev/null 2>&1; then
  Xvfb "$DISPLAY" -screen 0 960x544x24 &
  sleep 1
fi

export DISPLAY
export VITADECK_AUTO_ACTIVATE=default
export VITADECK_E2E_HOST_ECHO=1

INSTALLED="$OUT/vitadeck-data/installed-deck-apps"
mkdir -p "$INSTALLED"
rm -rf "$INSTALLED/default.vdapp"
cp -a "$ROOT/js/examples/default/dist/default.vdapp" "$INSTALLED/"

COMPANION_LOG="$(mktemp)"
VITADECK_LOG="$(mktemp)"
cleanup() {
  pkill -f "${OUT}/vitadeck" 2>/dev/null || true
  pkill -f "host-control-companion" 2>/dev/null || true
  rm -f "$COMPANION_LOG" "$VITADECK_LOG"
}
trap cleanup EXIT

cd "$ROOT/js/packages/host-control-companion"
pnpm start -- --vita "$VITA_URL" >"$COMPANION_LOG" 2>&1 &
sleep 3

cd "$OUT"
timeout 40 ./vitadeck >"$VITADECK_LOG" 2>&1 || true

if grep -q 'handled host.echo' "$COMPANION_LOG"; then
  echo "PASS: companion handled host.echo"
  exit 0
fi

echo "FAIL: host.echo not observed in companion log"
echo "--- companion ---"
tail -40 "$COMPANION_LOG"
echo "--- vitadeck ---"
tail -40 "$VITADECK_LOG"
exit 1
