#!/usr/bin/env bash
set -euo pipefail

ROOT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
TITLE_ID="PGRI00001"
VITA3K_HOME="${VITADECK_VITA3K_HOME:-$HOME/.vitadeck/vita3k}"
RESULT_TIMEOUT_S="${VITADECK_BENCH_TIMEOUT:-300}"
# shellcheck source=gui-run.sh
source "$ROOT_DIR/scripts/gui-run.sh"

usage() {
  cat <<'EOF'
usage: scripts/bench.sh <host|vita3k|vita> [--no-build] [--update-baseline] [scenario...]

Runs VitaDeck in Benchmark Mode (src/core/bench.c) and prints a report against tests/bench/baselines/<target>.
Without scenarios it runs all of them. Results go to out-bench/<target>-<time>/.

host     The desktop build in out/.
vita3k   The VPK in the Vita3K emulator (see the vitadeck-emulator skill).
vita     The VPK on the Vita at $PSVITAIP, through VitaShell's FTP (1337) and vitacompanion (1338).

host and vita3k wait for the machine-wide GUI lock (scripts/gui-run.sh) after building, and stop the run when no result
arrives in time.

--no-build          Run the existing build.
--update-baseline   Also copy the results to tests/bench/baselines/<target>; needs every scenario to run and pass.

Environment:
  PSVITAIP                 Vita IP, for vita
  VITADECK_VITA3K_HOME     Emulator config and storage (default: ~/.vitadeck/vita3k)
  VITADECK_BENCH_TIMEOUT   Seconds to wait for the result before stopping the run (default: 300)
EOF
}

step() {
  printf '\n==> %s\n' "$1"
}

die() {
  printf '%s\n' "$1" >&2
  exit 1
}

TARGET="${1:-}"
[[ "$TARGET" == "host" || "$TARGET" == "vita3k" || "$TARGET" == "vita" ]] || {
  usage >&2
  exit 1
}
shift
BUILD=1
UPDATE_BASELINE=0
SCENARIOS=()
for arg in "$@"; do
  case "$arg" in
    --no-build) BUILD=0 ;;
    --update-baseline) UPDATE_BASELINE=1 ;;
    -h | --help)
      usage
      exit 0
      ;;
    -*) die "Unknown option $arg" ;;
    *) SCENARIOS+=("$arg") ;;
  esac
done

# A baseline holds every scenario, so it is only replaced by a full run.
((UPDATE_BASELINE == 0 || ${#SCENARIOS[@]} == 0)) || die "--update-baseline needs a run of every scenario."
[[ "$TARGET" == "vita" ]] || "$ROOT_DIR/scripts/check-macos-display.sh"

RUN_DIR="$ROOT_DIR/out-bench/$TARGET-$(date +%Y%m%d-%H%M%S)"
mkdir -p "$RUN_DIR"
REQUEST="${SCENARIOS[*]:-}"

wait_for() {
  local description="$1"
  shift
  local deadline=$((SECONDS + RESULT_TIMEOUT_S))
  until "$@"; do
    ((SECONDS < deadline)) || die "Timed out waiting for $description."
    sleep 2
  done
}

# Runs the host or Vita3K until it quits after writing the result. Stops it on a timeout, or when it lingers after the
# result.
bench_gui() {
  local result="$1"
  shift
  gui_start "$@" >"$RUN_DIR/log.txt" 2>&1
  local deadline=$((SECONDS + RESULT_TIMEOUT_S)) linger=
  while kill -0 "$GUI_PID" 2>/dev/null; do
    if [[ -f "$result" ]]; then
      linger=${linger:-$((SECONDS + 10))}
      ((SECONDS < linger)) || break
    elif ((SECONDS >= deadline)); then
      gui_stop
      die "No result after ${RESULT_TIMEOUT_S}s (VITADECK_BENCH_TIMEOUT), so the run was stopped; see $RUN_DIR/log.txt"
    fi
    sleep 1
  done
  gui_stop
  [[ -f "$result" ]] || die "No result; see $RUN_DIR/log.txt"
}

build_js() {
  step "Build JS"
  pnpm --dir "$ROOT_DIR/js" build
}

build_vpk() {
  build_js
  step "Build VPK"
  (cd "$ROOT_DIR" && docker compose run --rm -T vitasdk bash -lc "cmake -S . -B out-vita && cmake --build out-vita")
}

run_host() {
  if ((BUILD)); then
    build_js
    step "Build host"
    cmake -S "$ROOT_DIR" -B "$ROOT_DIR/out" -G Ninja >/dev/null
    cmake --build "$ROOT_DIR/out" --target vitadeck stage_examples
  fi
  local data="$ROOT_DIR/out/vitadeck-data"
  gui_lock "$TARGET"
  mkdir -p "$data/bench"
  rm -f "$data/bench/result.json"
  printf '%s\n' "$REQUEST" >"$data/bench/request"
  step "Benchmark (host)"
  cd "$ROOT_DIR/out"
  VITADECK_DATA_ROOT="$data" bench_gui "$data/bench/result.json" ./vitadeck
  cp "$data/bench/result.json" "$RUN_DIR/result.json"
}

run_vita3k() {
  if ((BUILD)); then build_vpk; fi
  local data="$VITA3K_HOME/fs/ux0/data/vitadeck"
  gui_lock "$TARGET"
  mkdir -p "$data/bench"
  rm -f "$data/bench/result.json"
  printf '%s\n' "$REQUEST" >"$data/bench/request"
  step "Benchmark (Vita3K, quits with the app)"
  bench_gui "$data/bench/result.json" "$ROOT_DIR/scripts/vita3k.sh" launch
  cp "$data/bench/result.json" "$RUN_DIR/result.json"
}

vita_exists() {
  curl -sf --max-time 10 -o /dev/null "ftp://$PSVITAIP:1337/ux0:/data/vitadeck/bench/$1"
}

# Only a missing file (curl 78) counts; a timeout while the app starts does not.
vita_request_consumed() {
  local status=0
  vita_exists request || status=$?
  ((status == 78))
}

vita_result_written() {
  vita_exists result.json
}

# Prints curl config lines uploading the files in a local asset directory that the Vita lacks. The assets are pinned
# downloads, so a name already on the Vita is the same file. VitaShell's FTP has no NLST, so this parses LIST. The first
# run uploads about 4000 emoji, which takes most of an hour as VitaShell slows down with the directory's size.
missing_asset_uploads() {
  local dir="$ROOT_DIR/assets/$1" remote="ftp://$PSVITAIP:1337/ux0:/app/$TITLE_ID/assets/$1"
  [[ -d "$dir" ]] || return 0
  comm -23 <(ls "$dir" | sort) <(curl -s --max-time 60 "$remote/" | tr -d '\r' | awk '{ print $NF }' | sort) |
    while read -r name; do
      printf 'upload-file = "%s"\nurl = "%s/%s"\n' "$dir/$name" "$remote" "$name"
    done
}

run_vita() {
  [[ -n "${PSVITAIP:-}" ]] || die "Set PSVITAIP to the Vita's IP."
  if ((BUILD)); then build_vpk; fi
  local ftp="ftp://$PSVITAIP:1337/ux0:"
  local stage
  stage=$(mktemp -d)
  trap 'rm -rf "$stage"' RETURN

  step "Upload VitaDeck, its fonts and emoji, and the example Deck Apps"
  echo destroy | nc "$PSVITAIP" 1338 || true
  curl -sS -T "$ROOT_DIR/out-vita/eboot.bin" "$ftp/app/$TITLE_ID/"
  curl -sS -T "$ROOT_DIR/js/dist/runtime/runtime.js" "$ftp/app/$TITLE_ID/js/runtime.js"
  "$ROOT_DIR/scripts/stage-example-apps.sh" "$ROOT_DIR" "$stage" >/dev/null
  (
    cd "$stage"
    printf 'ftp-create-dirs\nsilent\nshow-error\n'
    find installed-deck-apps -type f | while read -r file; do
      printf 'upload-file = "%s"\nurl = "%s/data/vitadeck/%s"\n' "$stage/$file" "$ftp" "$file"
    done
    missing_asset_uploads fonts
    missing_asset_uploads emoji
  ) >"$stage/upload.curl"
  curl -K "$stage/upload.curl"

  step "Benchmark (Vita)"
  curl -s -o /dev/null -Q "-DELE result.json" "$ftp/data/vitadeck/bench/" || true
  printf '%s\n' "$REQUEST" | curl -sS --ftp-create-dirs -T - "$ftp/data/vitadeck/bench/request"
  echo launch "$TITLE_ID" | nc "$PSVITAIP" 1338
  wait_for "VitaDeck to start Benchmark Mode" vita_request_consumed
  wait_for "the result" vita_result_written
  curl -sS -o "$RUN_DIR/result.json" "$ftp/data/vitadeck/bench/result.json"
}

"run_$TARGET"

machine() {
  if [[ "$TARGET" == "vita" ]]; then
    echo "PS Vita"
  else
    echo "$(uname -sm) $( (sysctl -n machdep.cpu.brand_string 2>/dev/null || grep -m1 'model name' /proc/cpuinfo 2>/dev/null | cut -d: -f2) | sed 's/^ *//')"
  fi
}

cat >"$RUN_DIR/meta.json" <<EOF
{
  "target": "$TARGET",
  "commit": "$(git -C "$ROOT_DIR" rev-parse --short HEAD)",
  "dirty": $([[ -n "$(git -C "$ROOT_DIR" status --porcelain -- src js/packages js/examples CMakeLists.txt)" ]] && echo true || echo false),
  "date": "$(date -u +%Y-%m-%dT%H:%M:%SZ)",
  "machine": "$(machine)"
}
EOF

BASELINE="$ROOT_DIR/tests/bench/baselines/$TARGET"
step "Report"
pnpm --dir "$ROOT_DIR/js" --silent bench:report "$RUN_DIR" "$BASELINE" | tee "$RUN_DIR/report.txt"
echo "Results: $RUN_DIR"

if ((UPDATE_BASELINE)); then
  ! grep -q '"error"' "$RUN_DIR/result.json" || die "A scenario failed, so $BASELINE is unchanged."
  mkdir -p "$BASELINE"
  cp "$RUN_DIR/result.json" "$RUN_DIR/meta.json" "$BASELINE/"
  echo "Updated $BASELINE"
fi
