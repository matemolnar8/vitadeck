#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="$(cd "${1:?build directory required}" && pwd)"
shift
SOURCE_DIR="$(cd "$(dirname "$0")/.." && pwd)"
FIXTURE="$BUILD_DIR/smoke-fixture"
VDAPP_NAME="vitadeck-demo.vdapp"
VDAPP_SRC="$SOURCE_DIR/js/examples/demo/dist/$VDAPP_NAME"
UNAME="$(uname -s)"
GOLDEN_DIR="$SOURCE_DIR/tests/fixtures/golden/${UNAME}"
OUTPUT_DIR="$BUILD_DIR/smoke-screenshots"
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

rm -rf "$FIXTURE" "$OUTPUT_DIR"
mkdir -p "$FIXTURE/installed-deck-apps" "$OUTPUT_DIR"
INSTALLED="$FIXTURE/installed-deck-apps/$VDAPP_NAME"
cp -R "$VDAPP_SRC" "$INSTALLED"
cat "$SOURCE_DIR/tests/fixtures/demo_stubs.js" "$VDAPP_SRC/app.js" >"$INSTALLED/app.js"
printf '%s' "$VDAPP_NAME" >"$FIXTURE/active-package.txt"

# Relative, so the stack trace on the Failure Screen golden does not depend on the build directory.
cd "$BUILD_DIR"
export VITADECK_DATA_ROOT="smoke-fixture"

HARNESS=(./smoke_harness --golden-dir "$GOLDEN_DIR" --output-dir "$OUTPUT_DIR")
if [[ "$UPDATE_GOLDEN" -eq 1 || ! -d "$GOLDEN_DIR" ]]; then
  echo "smoke: writing golden images for ${UNAME} to ${GOLDEN_DIR}"
  rm -rf "$GOLDEN_DIR"
  mkdir -p "$GOLDEN_DIR"
  HARNESS+=(--update-golden)
fi

if [[ -z "${DISPLAY:-}" ]] && command -v xvfb-run >/dev/null 2>&1; then
  xvfb-run -a "${HARNESS[@]}"
else
  "${HARNESS[@]}"
fi
