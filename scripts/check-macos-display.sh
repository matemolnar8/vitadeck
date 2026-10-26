#!/usr/bin/env bash
# Fails fast before opening a macOS window: GLFW skips sleeping displays and then has no monitor, and Vita3K stalls on a
# locked screen. CGDisplayIsAsleep is the check GLFW makes.
set -euo pipefail

[[ "$(uname -s)" == Darwin ]] || exit 0

root=$(ioreg -n Root -d1)
asleep=$(osascript -l JavaScript -e 'ObjC.import("CoreGraphics"); $.CGDisplayIsAsleep($.CGMainDisplayID())' 2>/dev/null || true)
if [[ "$asleep" == 1 || "$asleep" == true ]] ||
  grep -Eq '"IOConsoleLocked" = Yes|"CGSSessionScreenIsLocked"=Yes' <<<"$root"; then
  echo "macOS display is asleep or the screen is locked; unlock it or use the Linux Docker path" >&2
  exit 1
fi
