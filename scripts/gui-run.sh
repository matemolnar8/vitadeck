#!/usr/bin/env bash
# One machine-wide lock for everything that opens the VitaDeck host window or Vita3K: concurrent GUI runs share
# ~/.vitadeck/vita3k and skew each other's timings. Scripts source this file; `scripts/gui-run.sh <command...>` runs a
# command under the lock. mkdir is the lock because it is atomic everywhere and macOS has no flock.

GUI_LOCK="${VITADECK_GUI_LOCK:-/tmp/vitadeck-gui.lock}"
GUI_PID=

gui_lock_owner() {
  cat "$GUI_LOCK/owner" 2>/dev/null || true
}

# Waits for the lock, then holds it until this script exits. A no-op when this script or a parent already holds it.
gui_lock() {
  local owner pid waiting=0
  owner=$(gui_lock_owner)
  pid=${owner%% *}
  [[ -n "$pid" && ("$pid" == "$$" || "$pid" == "${VITADECK_GUI_LOCK_PID:-}") ]] && return 0
  until mkdir "$GUI_LOCK" 2>/dev/null; do
    owner=$(gui_lock_owner)
    pid=${owner%% *}
    if [[ "$pid" =~ ^[0-9]+$ ]] && ! kill -0 "$pid" 2>/dev/null; then
      echo "Removing the stale GUI lock of $owner"
      rm -rf "$GUI_LOCK"
      continue
    fi
    if ((!waiting)); then
      echo "Waiting for the GUI lock $GUI_LOCK, held by ${owner:-an unknown process; remove it if nothing holds it}"
      waiting=1
    fi
    sleep 2
  done
  printf '%s %s in %s, since %s\n' "$$" "${0##*/} $*" "$PWD" "$(date +%H:%M:%S)" >"$GUI_LOCK/owner"
  export VITADECK_GUI_LOCK_PID=$$
  trap gui_cleanup EXIT
  trap 'exit 129' HUP
  trap 'exit 130' INT
  trap 'exit 143' TERM
}

gui_cleanup() {
  gui_stop
  local owner
  owner=$(gui_lock_owner)
  if [[ "${owner%% *}" == "$$" ]]; then rm -rf "$GUI_LOCK"; fi
}

# Starts a command in the background in its own process group, so gui_stop also stops everything it started.
gui_start() {
  set -m
  "$@" </dev/null &
  GUI_PID=$!
  set +m
}

# Stops the process group of gui_start: TERM, then KILL whatever is left after 10 s.
gui_stop() {
  [[ -n "$GUI_PID" ]] || return 0
  if kill -TERM -- "-$GUI_PID" 2>/dev/null; then
    local i
    for i in 1 2 3 4 5 6 7 8 9 10; do
      kill -0 -- "-$GUI_PID" 2>/dev/null || break
      sleep 1
    done
    kill -KILL -- "-$GUI_PID" 2>/dev/null || true
  fi
  wait "$GUI_PID" 2>/dev/null || true
  GUI_PID=
}

# Runs a command under the lock and returns its status. Under a parent's lock it runs in the foreground, inside the
# parent's process group, so the parent's gui_stop reaches it.
gui_run() {
  gui_lock "$@"
  if [[ "${VITADECK_GUI_LOCK_PID:-}" != "$$" ]]; then
    "$@"
    return
  fi
  gui_start "$@"
  local status=0
  wait "$GUI_PID" || status=$?
  GUI_PID=
  return "$status"
}

if [[ "${BASH_SOURCE[0]}" == "$0" ]]; then
  set -euo pipefail
  gui_run "$@"
fi
