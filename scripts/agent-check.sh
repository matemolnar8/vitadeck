#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT_DIR"

usage() {
    cat <<'EOF'
usage: scripts/agent-check.sh [fast|host|smoke|full]

fast   JS typecheck, CLI tests, JS lint, JS and C formatting checks
host   JS build, host configure/build, and non-smoke CTest
       (VITADECK_SANITIZE=address,undefined builds the host with sanitizers)
smoke  Dockerized Linux visual smoke CTest
full   fast, host, smoke, and Vita Docker build
EOF
}

step() {
    printf '\n==> %s\n' "$1"
}

die() {
    printf '%s\n' "$1" >&2
    exit 1
}

configure_host() {
    step "Configure host target"
    case "$(uname -s)" in
        Linux)
            CC=${CC:-gcc} CXX=${CXX:-g++} cmake -S . -B out -DCMAKE_EXE_LINKER_FLAGS="-Wl,--start-group" \
                -DVITADECK_SANITIZE="${VITADECK_SANITIZE:-}"
            ;;
        *)
            cmake -S . -B out -DVITADECK_SANITIZE="${VITADECK_SANITIZE:-}"
            ;;
    esac
}

run_fast() {
    step "Typecheck JS"
    pnpm --dir js tsc

    step "Test SDK CLI"
    pnpm --dir js test:cli

    step "Lint JS"
    pnpm --dir js lint

    step "Check JS formatting"
    pnpm --dir js format:check

    step "Check C formatting"
    ./scripts/clang-format.sh check
}

run_host() {
    step "Build JS"
    pnpm --dir js build

    configure_host

    step "Build host target"
    cmake --build out --parallel

    step "Run non-smoke CTest"
    ctest --test-dir out --output-on-failure -E smoke_harness
}

run_smoke_tests() {
    step "Run Dockerized Linux smoke CTest"
    scripts/smoke-docker.sh test
}

run_smoke() {
    run_smoke_tests
}

run_vita() {
    step "Check Docker daemon"
    docker info >/dev/null 2>&1 || die "Docker daemon not running; start Docker Desktop and rerun scripts/agent-check.sh full."

    step "Build Vita target"
    if docker compose version >/dev/null 2>&1; then
        compose="docker compose"
    elif command -v docker-compose >/dev/null 2>&1; then
        compose="docker-compose"
    else
        die "Neither docker compose nor docker-compose is available."
    fi
    $compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"
}

case "${1:-fast}" in
    fast)
        run_fast
        ;;
    host)
        run_host
        ;;
    smoke)
        run_smoke
        ;;
    full)
        run_fast
        run_host
        run_smoke_tests
        run_vita
        ;;
    -h|--help|help)
        usage
        ;;
    *)
        usage >&2
        exit 2
        ;;
esac
