#!/usr/bin/env bash
set -euo pipefail

usage() {
  cat <<'EOF'
Usage: upload-vdapp.sh [--ip ADDR] [--port PORT] ARCHIVE.zip

Upload a Runtime Upload Archive to VitaDeck on a Vita.
Requires VitaDeck Shell → Upload to be active on the device.

Options:
  --ip ADDR    Vita IP (default: resolve from PSVITAIP / CMake cache / CMakeLists.txt)
  --port PORT  Upload listener port (default: 8787)
EOF
}

resolve_from_cmake_cache() {
  local cache="$1"
  [[ -f "$cache" ]] || return 1
  sed -n 's/^PSVITAIP:STRING=\(.*\)$/\1/p' "$cache" | head -1
}

resolve_from_cmake_lists() {
  local lists="$1"
  [[ -f "$lists" ]] || return 1
  sed -n 's/^set(PSVITAIP "\([^"]*\)".*/\1/p' "$lists" | head -1
}

resolve_vita_ip() {
  if [[ -n "${PSVITAIP:-}" ]]; then
    printf '%s' "$PSVITAIP"
    return
  fi

  local script_dir repo_root ip
  script_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
  repo_root="$(cd "$script_dir/../../../.." && pwd)"

  for cache in "$repo_root/out-vita/CMakeCache.txt" "$repo_root/out/CMakeCache.txt"; do
    ip="$(resolve_from_cmake_cache "$cache" || true)"
    if [[ -n "$ip" ]]; then
      printf '%s' "$ip"
      return
    fi
  done

  ip="$(resolve_from_cmake_lists "$repo_root/CMakeLists.txt" || true)"
  if [[ -n "$ip" ]]; then
    printf '%s' "$ip"
    return
  fi

  echo "upload-vdapp.sh: could not resolve Vita IP; pass --ip or set PSVITAIP" >&2
  exit 1
}

ip=""
port="8787"
archive=""

while [[ $# -gt 0 ]]; do
  case "$1" in
    --ip)
      ip="${2:?--ip requires an address}"
      shift 2
      ;;
    --port)
      port="${2:?--port requires a number}"
      shift 2
      ;;
    -h|--help)
      usage
      exit 0
      ;;
    --)
      shift
      break
      ;;
    -*)
      echo "upload-vdapp.sh: unknown option: $1" >&2
      usage >&2
      exit 1
      ;;
    *)
      archive="$1"
      shift
      ;;
  esac
done

if [[ -z "$archive" && $# -gt 0 ]]; then
  archive="$1"
fi

if [[ -z "$archive" ]]; then
  usage >&2
  exit 1
fi

if [[ ! -f "$archive" ]]; then
  echo "upload-vdapp.sh: archive not found: $archive" >&2
  exit 1
fi

if [[ -z "$ip" ]]; then
  ip="$(resolve_vita_ip)"
fi

url="http://${ip}:${port}/upload"
echo "Uploading $(basename "$archive") to ${url}" >&2

response="$(curl -fsS -m 30 -X POST "$url" -F "archive=@${archive}")"
printf '%s\n' "$response"

if ! printf '%s' "$response" | grep -q '"ok"[[:space:]]*:[[:space:]]*true'; then
  exit 1
fi
