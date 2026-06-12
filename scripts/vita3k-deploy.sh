#!/usr/bin/env bash
set -euo pipefail

TITLE_ID="${VITA_TITLE_ID:-PGRI00001}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_VITA="${REPO_ROOT}/out-vita"
VPK="${OUT_VITA}/vitadeck.vpk"

if [[ "$(uname -s)" == "Darwin" ]]; then
  VITA3K_PREF="${VITA3K_PREF:-${HOME}/Library/Application Support/Vita3K/Vita3K}"
else
  VITA3K_PREF="${VITA3K_PREF:-${HOME}/.local/share/Vita3K/Vita3K}"
fi

APP_DIR="${VITA3K_PREF}/ux0/app/${TITLE_ID}"
DATA_ROOT="${VITA3K_PREF}/ux0/data/vitadeck"
RUNTIME_SRC="${REPO_ROOT}/js/dist/runtime/runtime.js"
FONT_SRC="${REPO_ROOT}/assets/fonts/DejaVuSans.ttf"
FONT_LICENSE_SRC="${REPO_ROOT}/assets/fonts/DejaVuSans-LICENSE.txt"

usage() {
  cat <<EOF
Usage: $(basename "$0") <command>

Commands:
  install-vpk     Unzip out-vita/vitadeck.vpk into Vita3K ux0/app/${TITLE_ID}
  sync            Copy eboot.bin, js/runtime.js, and fonts (fast rebuild deploy)
  seed-deck-app   Copy js/dist/deck-app and set active-package.txt
                  Optional arg: package folder name (default: chat.vdapp from deck-app)

Environment:
  VITA3K_PREF     Emulator pref-path (default: OS-specific Vita3K data dir)
  VITA_TITLE_ID   Title ID (default: PGRI00001)
EOF
}

require_vpk() {
  if [[ ! -f "${VPK}" ]]; then
    echo "Missing ${VPK}. Build the Vita target first (see vitadeck-build skill)." >&2
    exit 1
  fi
}

require_runtime() {
  if [[ ! -f "${RUNTIME_SRC}" ]]; then
    echo "Missing ${RUNTIME_SRC}. Run: pnpm --dir js build" >&2
    exit 1
  fi
}

install_vpk() {
  require_vpk
  mkdir -p "${APP_DIR}"
  unzip -o "${VPK}" -d "${APP_DIR}"
  echo "Installed VPK to ${APP_DIR}"
}

sync_app() {
  require_runtime
  if [[ ! -f "${OUT_VITA}/eboot.bin" ]]; then
    echo "Missing ${OUT_VITA}/eboot.bin. Build the Vita target first." >&2
    exit 1
  fi
  mkdir -p "${APP_DIR}/js" "${APP_DIR}/assets/fonts"
  cp "${OUT_VITA}/eboot.bin" "${APP_DIR}/eboot.bin"
  cp "${RUNTIME_SRC}" "${APP_DIR}/js/runtime.js"
  cp "${FONT_SRC}" "${APP_DIR}/assets/fonts/DejaVuSans.ttf"
  cp "${FONT_LICENSE_SRC}" "${APP_DIR}/assets/fonts/DejaVuSans-LICENSE.txt"
  echo "Synced app binaries to ${APP_DIR}"
}

seed_deck_app() {
  require_runtime
  local package_name="${1:-chat.vdapp}"
  local deck_src="${REPO_ROOT}/js/dist/deck-app"
  local dest="${DATA_ROOT}/installed-deck-apps/${package_name}"

  if [[ ! -d "${deck_src}" ]]; then
    echo "Missing ${deck_src}. Run: pnpm --dir js build" >&2
    exit 1
  fi

  mkdir -p "${DATA_ROOT}/installed-deck-apps"
  rm -rf "${dest}"
  cp -R "${deck_src}" "${dest}"
  printf '%s\n' "${package_name}" > "${DATA_ROOT}/active-package.txt"
  echo "Seeded ${dest} and set active-package.txt"
}

cmd="${1:-}"
case "${cmd}" in
  install-vpk) install_vpk ;;
  sync) sync_app ;;
  seed-deck-app) seed_deck_app "${2:-}" ;;
  -h | --help | help | "") usage ;;
  *)
    echo "Unknown command: ${cmd}" >&2
    usage
    exit 1
    ;;
esac
