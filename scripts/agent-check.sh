#!/bin/sh
set -eu

ROOT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")/.." && pwd)
cd "$ROOT_DIR"

usage() {
    cat <<'EOF'
usage: scripts/agent-check.sh [fast|host|smoke|full]

fast   JS install and build, JS typecheck, CLI tests, JS lint, JS and C formatting checks
host   JS install and build, host configure/build, and non-smoke CTest
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

# A fresh worktree often starts on the shell's default Node, which is older than .nvmrc.
use_pinned_node() {
    want=$(cat .nvmrc)
    have=$(node -p 'process.versions.node.split(".")[0]' 2>/dev/null || echo none)
    [ "$have" = "$want" ] && return
    if [ -z "${VITADECK_PINNED_NODE:-}" ] && command -v fnm >/dev/null 2>&1; then
        printf 'Node %s is active; rerunning on Node %s from .nvmrc with fnm.\n' "$have" "$want" >&2
        VITADECK_PINNED_NODE=1 exec fnm exec --using="$ROOT_DIR/.nvmrc" -- "$ROOT_DIR/scripts/agent-check.sh" "$@"
    fi
    die "Node $have is active, but VitaDeck needs Node $want (.nvmrc). Run fnm use or nvm use, then rerun."
}

install_js() {
    step "Install JS dependencies"
    pnpm --dir js install --frozen-lockfile
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
    install_js

    # tsc reads the SDK's dist and the font and image types `vitadeck build` writes to each example's .vitadeck/,
    # so a fresh worktree fails it with unrelated errors such as '"mono"' is not assignable.
    step "Build JS"
    pnpm --dir js build

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
    install_js

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
    docker compose run --rm vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita"
}

case "${1:-fast}" in
    fast | host | full) use_pinned_node "$@" ;;
esac

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
