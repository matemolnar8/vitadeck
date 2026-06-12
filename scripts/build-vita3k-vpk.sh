#!/usr/bin/env bash
set -euo pipefail

REPO_ROOT="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
OUT_DIR="${REPO_ROOT}/out-vita3k"
COMPOSE=(docker compose)

docker_cmd() {
  if docker info >/dev/null 2>&1; then
    DOCKER=(docker)
  elif sudo docker info >/dev/null 2>&1; then
    DOCKER=(sudo docker)
  else
    echo "Docker daemon is not running." >&2
    exit 1
  fi
}

if ! command -v docker >/dev/null 2>&1; then
  echo "Docker is required to build a Vita3K-target VPK." >&2
  echo "Install Docker locally, or wait for CI and run:" >&2
  echo "  scripts/vita3k-deploy.sh download-vpk" >&2
  exit 1
fi

docker_cmd

if ! "${DOCKER[@]}" compose version >/dev/null 2>&1; then
  if command -v docker-compose >/dev/null 2>&1; then
    COMPOSE=(docker-compose)
  else
    echo "docker compose or docker-compose is required." >&2
    exit 1
  fi
else
  COMPOSE=("${DOCKER[@]}" compose)
fi

if [[ ! -f "${REPO_ROOT}/js/dist/runtime/runtime.js" ]]; then
  echo "Missing js/dist/runtime/runtime.js — run: pnpm --dir js build" >&2
  exit 1
fi

cd "${REPO_ROOT}"
"${COMPOSE[@]}" run --rm vitasdk-vita3k bash -lc \
  "cmake -S . -B out-vita3k && cmake --build out-vita3k --parallel \"\$(nproc)\""

if [[ ! -f "${OUT_DIR}/vitadeck.vpk" ]]; then
  echo "Build finished but ${OUT_DIR}/vitadeck.vpk is missing." >&2
  exit 1
fi

echo "Built Vita3K-target VPK: ${OUT_DIR}/vitadeck.vpk"
