#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT_DIR"

usage() {
    cat <<'EOF'
usage: scripts/agent-check.sh [fast|host|smoke|full]

fast   JS typecheck, JS lint, and C formatting check
host   JS build, host configure/build, and non-smoke CTest
smoke  host build plus visual smoke CTest
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
            CC=${CC:-gcc} CXX=${CXX:-g++} cmake -S . -B out -DCMAKE_EXE_LINKER_FLAGS="-Wl,--start-group"
            ;;
        *)
            cmake -S . -B out
            ;;
    esac
}

run_fast() {
    step "Typecheck JS"
    pnpm --dir js tsc

    step "Lint JS"
    pnpm --dir js lint

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
    step "Run smoke CTest"
    ctest --test-dir out --output-on-failure -R smoke_harness
}

run_smoke() {
    run_host
    run_smoke_tests
}

run_vita() {
    step "Check Docker daemon"
    docker info >/dev/null 2>&1 || die "Docker daemon not running; start Docker Desktop and rerun scripts/agent-check.sh full."

    step "Build Vita target"
    docker-compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"
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
