#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="${1:?build directory required}"
SOURCE_DIR="$(cd "$(dirname "$0")/.." && pwd)"
FIXTURE="$BUILD_DIR/smoke-fixture"
VDAPP_SRC="$SOURCE_DIR/js/examples/smoke/dist/smoke.vdapp"
GOLDEN="$SOURCE_DIR/tests/fixtures/smoke_golden.png"
OUTPUT="$BUILD_DIR/smoke_screenshot.png"

if [[ ! -d "$VDAPP_SRC" ]]; then
  echo "smoke fixture missing: $VDAPP_SRC (run pnpm --dir js build first)" >&2
  exit 1
fi

rm -rf "$FIXTURE"
mkdir -p "$FIXTURE/installed-deck-apps"
cp -R "$VDAPP_SRC" "$FIXTURE/installed-deck-apps/smoke.vdapp"
printf '%s' "smoke.vdapp" >"$FIXTURE/active-package.txt"

cd "$BUILD_DIR"
export VITADECK_DATA_ROOT="$FIXTURE"

HARNESS=(./smoke_harness --golden "$GOLDEN" --output "$OUTPUT")
if [[ -z "${DISPLAY:-}" ]] && command -v xvfb-run >/dev/null 2>&1; then
  exec xvfb-run -a "${HARNESS[@]}"
fi
exec "${HARNESS[@]}"
