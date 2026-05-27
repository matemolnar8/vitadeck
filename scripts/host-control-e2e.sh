#!/usr/bin/env bash
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
OUT="${VITADECK_OUT:-$ROOT/out}"
DISPLAY="${DISPLAY:-:99}"
VITA_URL="${VITA_URL:-http://127.0.0.1:8787}"
CALLBACK_HOST="${CALLBACK_HOST:-127.0.0.1}"

if ! pgrep -f "Xvfb $DISPLAY" >/dev/null 2>&1; then
  Xvfb "$DISPLAY" -screen 0 960x544x24 &
  sleep 1
fi

export DISPLAY
export VITADECK_AUTO_ACTIVATE=default
export VITADECK_E2E_HOST_ECHO=1

INSTALLED="$OUT/vitadeck-data/installed-deck-apps"
mkdir -p "$INSTALLED"
rm -f "$OUT/vitadeck-data/host-control-link.json"
rm -rf "$INSTALLED/default.vdapp"
cp -a "$ROOT/js/examples/default/dist/default.vdapp" "$INSTALLED/"

COMPANION_LOG="$(mktemp)"
VITADECK_LOG="$(mktemp)"
cleanup() {
  pkill -f "${OUT}/vitadeck" 2>/dev/null || true
  pkill -f "host-control-companion" 2>/dev/null || true
  pkill -f "tsx src/cli.ts" 2>/dev/null || true
  rm -f "$COMPANION_LOG" "$VITADECK_LOG"
}
trap cleanup EXIT

cd "$OUT"
./vitadeck >"$VITADECK_LOG" 2>&1 &
sleep 2

cd "$ROOT/js/packages/host-control-companion"
pnpm start -- --vita "$VITA_URL" --callback-host "$CALLBACK_HOST" >"$COMPANION_LOG" 2>&1 &

for _ in $(seq 1 30); do
  if grep -q '\[host-control\] linked to Vita' "$COMPANION_LOG"; then
    break
  fi
  sleep 1
done

if ! grep -q '\[host-control\] linked to Vita' "$COMPANION_LOG"; then
  echo "FAIL: companion did not link within 30s"
  tail -40 "$COMPANION_LOG"
  exit 1
fi

STATUS_JSON="$(curl -sf "$VITA_URL/v1/host/status" || true)"
CALLBACK_URL="$(printf '%s' "$STATUS_JSON" | sed -n 's/.*"callbackUrl":"\([^"]*\)".*/\1/p')"
if [ -n "$CALLBACK_URL" ]; then
  CURL_RESULT="$(curl -sf -X POST "$CALLBACK_URL/v1/command" \
    -H 'Content-Type: application/json' \
    -d '{"command":"host.echo","payload":{"from":"e2e-curl"}}' || true)"
  if printf '%s' "$CURL_RESULT" | grep -q '"ok":true'; then
    echo "PASS: link, status, and host command path OK (curl)"
    if grep -q 'handled host.echo' "$COMPANION_LOG"; then
      echo "PASS: deck-app host.echo also observed"
    fi
    exit 0
  fi
fi

for _ in $(seq 1 35); do
  if grep -q 'handled host.echo' "$COMPANION_LOG"; then
    echo "PASS: companion linked and handled host.echo (deck app)"
    exit 0
  fi
  sleep 1
done

echo "FAIL: host.echo not observed in companion log"
echo "--- companion ---"
tail -40 "$COMPANION_LOG"
echo "--- vitadeck ---"
tail -40 "$VITADECK_LOG"
exit 1
