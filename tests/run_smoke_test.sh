#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="$(cd "${1:?build directory required}" && pwd)"
shift
SOURCE_DIR="$(cd "$(dirname "$0")/.." && pwd)"
FIXTURE="$BUILD_DIR/smoke-fixture"
VDAPP_SRC="$SOURCE_DIR/js/examples/smoke/dist/smoke.vdapp"
UNAME="$(uname -s)"
GOLDEN="$SOURCE_DIR/tests/fixtures/smoke_golden.${UNAME}.png"
OUTPUT="$BUILD_DIR/smoke_screenshot.png"
UPDATE_GOLDEN=0

while [[ $# -gt 0 ]]; do
  case "$1" in
    --update-golden)
      UPDATE_GOLDEN=1
      shift
      ;;
    -h|--help)
      echo "usage: tests/run_smoke_test.sh BUILD_DIR [--update-golden]"
      exit 0
      ;;
    *)
      echo "unknown smoke test option: $1" >&2
      exit 2
      ;;
  esac
done

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
if [[ "$UPDATE_GOLDEN" -eq 1 ]]; then
  echo "smoke: updating golden image for ${UNAME} at ${GOLDEN}"
  HARNESS+=(--update-golden)
elif [[ ! -f "$GOLDEN" ]]; then
  echo "smoke: bootstrapping golden image for ${UNAME} at ${GOLDEN}"
  HARNESS+=(--update-golden)
fi

if [[ -z "${DISPLAY:-}" ]] && command -v xvfb-run >/dev/null 2>&1; then
  xvfb-run -a "${HARNESS[@]}"
else
  "${HARNESS[@]}"
fi
