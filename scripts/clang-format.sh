#!/usr/bin/env bash
set -euo pipefail

root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
cd "$root"

if ! command -v clang-format >/dev/null 2>&1; then
    echo "clang-format not found; install LLVM clang-format (CI pins 21.1.8: pipx install clang-format==21.1.8)" >&2
    exit 1
fi

mode="${1:-check}"
case "$mode" in
check)
    find src tests \( -name '*.c' -o -name '*.h' \) -print | sort | xargs clang-format --dry-run --Werror
    ;;
format)
    find src tests \( -name '*.c' -o -name '*.h' \) -print | sort | xargs clang-format -i
    ;;
*)
    echo "usage: $0 [check|format]" >&2
    exit 1
    ;;
esac
