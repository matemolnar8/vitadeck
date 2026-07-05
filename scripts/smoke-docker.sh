#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
IMAGE="${VITADECK_SMOKE_IMAGE:-vitadeck-smoke}"
BUILD_DIR="${VITADECK_SMOKE_BUILD_DIR:-out-smoke}"
PNPM_INSTALL_ARGS="${VITADECK_SMOKE_PNPM_INSTALL_ARGS:---frozen-lockfile}"

usage() {
  cat <<'EOF'
usage: scripts/smoke-docker.sh [test|update-golden|shell]

test          Build and run the Linux smoke test.
update-golden Build and refresh tests/fixtures/smoke_golden.Linux.png.
shell         Open a shell in the smoke Docker image.

Environment:
  VITADECK_SMOKE_IMAGE       Docker image tag to build/use (default: vitadeck-smoke)
  VITADECK_SMOKE_BUILD_DIR   Build directory inside the repo (default: out-smoke)
  VITADECK_SMOKE_NO_BUILD    Set to 1 to skip docker build
EOF
}

step() {
  printf '\n==> %s\n' "$1"
}

run_inside() {
  local mode="$1"
  cd /build/git

  export PNPM_HOME=/pnpm
  export PATH="$PNPM_HOME:$PATH"
  pnpm config set store-dir /pnpm/store

  local pnpm_args=()
  if [[ -n "$PNPM_INSTALL_ARGS" ]]; then
    read -r -a pnpm_args <<<"$PNPM_INSTALL_ARGS"
  fi

  step "Install JS dependencies"
  pnpm --dir js install "${pnpm_args[@]}"

  step "Build JS runtime and smoke app"
  pnpm --dir js build

  step "Configure host smoke build"
  CC=gcc CXX=g++ cmake -S . -B "$BUILD_DIR" -DCMAKE_EXE_LINKER_FLAGS="-Wl,--start-group"

  step "Build smoke harness"
  cmake --build "$BUILD_DIR" --parallel "$(nproc)" --target smoke_harness

  case "$mode" in
    test)
      step "Run smoke test"
      ctest --test-dir "$BUILD_DIR" --output-on-failure -R smoke_harness
      ;;
    update-golden)
      step "Update Linux smoke golden"
      tests/run_smoke_test.sh "$BUILD_DIR" --update-golden
      ;;
    *)
      usage >&2
      exit 2
      ;;
  esac

  if [[ -n "${LOCAL_UID:-}" && -n "${LOCAL_GID:-}" ]]; then
    chown -R "$LOCAL_UID:$LOCAL_GID" "$BUILD_DIR" js/dist js/examples/smoke/dist tests/fixtures/smoke_golden.Linux.png 2>/dev/null || true
  fi
}

run_outside() {
  local mode="$1"
  cd "$ROOT_DIR"

  if [[ "${VITADECK_SMOKE_NO_BUILD:-0}" != "1" ]]; then
    step "Build smoke Docker image"
    docker build --platform linux/amd64 --target smoke -t "$IMAGE" -f Dockerfile .
  fi

  local volume_suffix
  volume_suffix="$(basename "$ROOT_DIR" | tr -c '[:alnum:]_.-' '-')"

  if [[ "$mode" == "shell" ]]; then
    docker run --rm -it --platform linux/amd64 \
      -v "$ROOT_DIR:/build/git" \
      --mount "type=volume,source=vitadeck-smoke-${volume_suffix}-node-modules,target=/build/git/js/node_modules" \
      --mount "type=volume,source=vitadeck-smoke-${volume_suffix}-pnpm-store,target=/pnpm/store" \
      -w /build/git \
      -e VITADECK_SMOKE_BUILD_DIR="$BUILD_DIR" \
      -e VITADECK_SMOKE_PNPM_INSTALL_ARGS="$PNPM_INSTALL_ARGS" \
      -e LOCAL_UID="$(id -u)" \
      -e LOCAL_GID="$(id -g)" \
      "$IMAGE" bash
    return
  fi

  step "Run smoke workflow in Docker"
  docker run --rm --platform linux/amd64 \
    -v "$ROOT_DIR:/build/git" \
    --mount "type=volume,source=vitadeck-smoke-${volume_suffix}-node-modules,target=/build/git/js/node_modules" \
    --mount "type=volume,source=vitadeck-smoke-${volume_suffix}-pnpm-store,target=/pnpm/store" \
    -w /build/git \
    -e VITADECK_SMOKE_BUILD_DIR="$BUILD_DIR" \
    -e VITADECK_SMOKE_PNPM_INSTALL_ARGS="$PNPM_INSTALL_ARGS" \
    -e LOCAL_UID="$(id -u)" \
    -e LOCAL_GID="$(id -g)" \
    "$IMAGE" scripts/smoke-docker.sh --inside "$mode"
}

if [[ "${1:-}" == "--inside" ]]; then
  shift
  run_inside "${1:-test}"
else
  case "${1:-test}" in
    test|update-golden|shell)
      run_outside "${1:-test}"
      ;;
    -h|--help|help)
      usage
      ;;
    *)
      usage >&2
      exit 2
      ;;
  esac
fi
