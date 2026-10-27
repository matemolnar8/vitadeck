#!/usr/bin/env bash
set -euo pipefail

BUILD_DIR="$(cd "${1:?build directory required}" && pwd)"
SOURCE_DIR="$(cd "$(dirname "$0")/.." && pwd)"
FIXTURE="$BUILD_DIR/deck-app-input-fixture"

rm -rf "$FIXTURE"
"$SOURCE_DIR/scripts/stage-example-apps.sh" "$SOURCE_DIR" "$FIXTURE" >/dev/null

cd "$BUILD_DIR"
export VITADECK_DATA_ROOT="$FIXTURE"

if [[ -z "${DISPLAY:-}" ]] && command -v xvfb-run >/dev/null 2>&1; then
  xvfb-run -a ./deck_app_input_harness
else
  ./deck_app_input_harness
fi
