#!/usr/bin/env bash
set -euo pipefail

TITLE_ID="${VITA_TITLE_ID:-PGRI00001}"
REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_VITA3K="${REPO_ROOT}/out-vita3k"
VPK="${OUT_VITA3K}/vitadeck.vpk"
VITA3K_BIN="${VITA3K_BIN:-}"

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
  build-vpk       Build out-vita3k/vitadeck.vpk (vitasdk-vita3k Docker image)
  download-vpk    Fetch vitadeck-vpk-vita3k-* CI artifact (cloud agents without Docker)
  setup-vita3k    Fix shader paths for extracted AppImage (once per install)
  install-vpk     Unzip out-vita3k/vitadeck.vpk into Vita3K ux0/app/${TITLE_ID}
  sync            Copy eboot.bin, js/runtime.js, and fonts (fast rebuild deploy)
  seed-deck-app   Copy js/dist/deck-app and set active-package.txt
                  Optional arg: package name (default: chat → chat.vdapp)

Environment:
  VITA3K_PREF     Emulator pref-path (default: OS-specific Vita3K data dir)
  VITA3K_BIN      Path to Vita3K binary (for setup-vita3k shader symlink)
  VITA_TITLE_ID   Title ID (default: PGRI00001)
EOF
}

require_vpk() {
  if [[ ! -f "${VPK}" ]]; then
    echo "Missing ${VPK}." >&2
    echo "Build a Vita3K-target VPK: $(basename "$0") build-vpk" >&2
    echo "Or download from CI:      $(basename "$0") download-vpk" >&2
    exit 1
  fi
}

require_runtime() {
  if [[ ! -f "${RUNTIME_SRC}" ]]; then
    echo "Missing ${RUNTIME_SRC}. Run: pnpm --dir js build" >&2
    exit 1
  fi
}

normalize_package_name() {
  local name="${1:-chat.vdapp}"
  if [[ "${name}" != *.vdapp ]]; then
    name="${name}.vdapp"
  fi
  printf '%s' "${name}"
}

build_vpk() {
  "${REPO_ROOT}/scripts/build-vita3k-vpk.sh"
}

download_vpk() {
  if ! command -v gh >/dev/null 2>&1; then
    echo "gh CLI is required to download CI artifacts." >&2
    exit 1
  fi

  local run_id="${1:-}"
  if [[ -z "${run_id}" ]]; then
    run_id="$(gh run list --repo "$(gh repo view --json nameWithOwner -q .nameWithOwner 2>/dev/null || echo matemolnar8/vitadeck)" \
      --workflow="Vita (VPK)" --limit 30 --json databaseId,conclusion \
      -q '.[] | select(.conclusion=="success") | .databaseId' | head -1)"
  fi
  if [[ -z "${run_id}" ]]; then
    echo "No successful Vita (VPK) workflow run found." >&2
    exit 1
  fi

  local artifact
  artifact="$(gh api "repos/matemolnar8/vitadeck/actions/runs/${run_id}/artifacts" \
    --jq '.artifacts[] | select(.name | startswith("vitadeck-vpk-vita3k")) | .name' | head -1)"
  if [[ -z "${artifact}" ]]; then
    echo "No vitadeck-vpk-vita3k artifact on run ${run_id}." >&2
    echo "Do not use vitadeck-vpk-* (hardware) artifacts for Vita3K testing." >&2
    exit 1
  fi

  mkdir -p "${OUT_VITA3K}"
  gh run download "${run_id}" -n "${artifact}" -D "${OUT_VITA3K}"
  if [[ ! -f "${VPK}" ]]; then
    echo "Download finished but ${VPK} is missing." >&2
    exit 1
  fi
  echo "Downloaded ${VPK} from run ${run_id} (${artifact})"
}

setup_vita3k() {
  local bin="${VITA3K_BIN:-${HOME}/vita3k/squashfs-root/usr/bin/Vita3K}"
  local bin_dir
  bin_dir="$(dirname "${bin}")"
  local share_shaders="${bin_dir%/bin}/share/Vita3K/shaders-builtin"
  if [[ ! -d "${share_shaders}" ]]; then
    share_shaders="$(dirname "${bin}")/shaders-builtin"
  fi
  if [[ ! -d "${share_shaders}" ]]; then
    echo "Could not find shaders-builtin near ${bin}. Set VITA3K_BIN." >&2
    exit 1
  fi
  ln -sfn "${share_shaders}" "${bin_dir}/shaders-builtin"
  echo "Linked ${bin_dir}/shaders-builtin -> ${share_shaders}"
}

install_vpk() {
  require_vpk
  mkdir -p "${APP_DIR}"
  unzip -o "${VPK}" -d "${APP_DIR}"
  echo "Installed VPK to ${APP_DIR}"
}

sync_app() {
  require_runtime
  if [[ ! -f "${OUT_VITA3K}/eboot.bin" ]]; then
    echo "Missing ${OUT_VITA3K}/eboot.bin. Re-run build-vpk or install-vpk." >&2
    exit 1
  fi
  mkdir -p "${APP_DIR}/js" "${APP_DIR}/assets/fonts"
  cp "${OUT_VITA3K}/eboot.bin" "${APP_DIR}/eboot.bin"
  cp "${RUNTIME_SRC}" "${APP_DIR}/js/runtime.js"
  cp "${FONT_SRC}" "${APP_DIR}/assets/fonts/DejaVuSans.ttf"
  cp "${FONT_LICENSE_SRC}" "${APP_DIR}/assets/fonts/DejaVuSans-LICENSE.txt"
  echo "Synced app binaries to ${APP_DIR}"
}

seed_deck_app() {
  require_runtime
  local package_name
  package_name="$(normalize_package_name "${1:-chat.vdapp}")"
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
  build-vpk) build_vpk ;;
  download-vpk) download_vpk "${2:-}" ;;
  setup-vita3k) setup_vita3k ;;
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
