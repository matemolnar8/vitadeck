#!/usr/bin/env bash
# Drives the VitaDeck host window for manual tests and recordings when computerUse is not available: Linux X11
# (including a private Xvfb display) through xdotool and ffmpeg, macOS through a small Swift helper.
set -euo pipefail

usage() {
  cat <<'EOF'
usage: scripts/gui-drive.sh key <key>...     press and release each key: F1, Return, BackSpace, Up, Down, Left, Right, a-z
       scripts/gui-drive.sh click <x> <y>    left-click at a point in the window (window points, top left is 0 0)
       scripts/gui-drive.sh shot <file.png>  screenshot the window
       scripts/gui-drive.sh rect             print the window's screen rect as x,y,width,height

Linux: drives the VitaDeck window on $DISPLAY. Needs xdotool and ffmpeg.
macOS: needs Xcode's swiftc, and Accessibility access for the app that runs this script (System Settings, Privacy
& Security, Accessibility); without it, keys and clicks are dropped silently.
EOF
}

die() {
  echo "gui-drive: $*" >&2
  exit 1
}

# GLFW drops synthetic events aimed at an unfocused window, so `xdotool key --window` and `xdotool click` do nothing.
# Focus the window, then send separate down and up events.
linux() {
  local cmd="$1"
  shift
  [[ -n "${DISPLAY:-}" ]] || die "DISPLAY is not set"
  local win
  win=$(xdotool search --name '^VitaDeck$' | head -1)
  [[ -n "$win" ]] || die "no VitaDeck window on $DISPLAY"
  local X Y WIDTH HEIGHT SCREEN WINDOW
  eval "$(xdotool getwindowgeometry --shell "$win")"
  case "$cmd" in
    key)
      xdotool windowfocus "$win"
      sleep 0.1
      local key
      for key in "$@"; do
        xdotool keydown "$key"
        sleep 0.08
        xdotool keyup "$key"
        sleep 0.15
      done
      ;;
    click)
      xdotool windowfocus "$win" mousemove --window "$win" "$1" "$2" mousedown 1 sleep 0.12 mouseup 1
      ;;
    shot)
      ffmpeg -loglevel error -y -f x11grab -draw_mouse 0 -video_size "${WIDTH}x${HEIGHT}" -i "$DISPLAY+$X,$Y" \
        -frames:v 1 "$1"
      ;;
    rect)
      echo "$X,$Y,$WIDTH,$HEIGHT"
      ;;
  esac
}

mac_helper() {
  local helper
  helper="${TMPDIR:-/tmp}/vitadeck-gui-drive-$(cksum <"${BASH_SOURCE[0]}" | cut -d' ' -f1)"
  if [[ ! -x "$helper" ]]; then
    local source="$helper.swift"
    cat >"$source" <<'EOF'
import AppKit
import CoreGraphics

let args = CommandLine.arguments
let window = (CGWindowListCopyWindowInfo([.optionOnScreenOnly], kCGNullWindowID) as? [[String: Any]] ?? [])
  .first { ($0[kCGWindowOwnerName as String] as? String) == "vitadeck" }
guard let window, let id = window[kCGWindowNumber as String] as? Int,
  let bounds = CGRect(dictionaryRepresentation: window[kCGWindowBounds as String] as! CFDictionary)
else {
  FileHandle.standardError.write("gui-drive: no VitaDeck window\n".data(using: .utf8)!)
  exit(1)
}

func activate() {
  NSWorkspace.shared.runningApplications.first { $0.localizedName == "vitadeck" }?
    .activate(options: [.activateIgnoringOtherApps])
  usleep(300_000)
}

let source = CGEventSource(stateID: .hidSystemState)
switch args[1] {
case "winid":
  print(id)
case "rect":
  print("\(Int(bounds.minX)),\(Int(bounds.minY)),\(Int(bounds.width)),\(Int(bounds.height))")
case "key":
  activate()
  for code in args.dropFirst(2) {
    for down in [true, false] {
      CGEvent(keyboardEventSource: source, virtualKey: CGKeyCode(code)!, keyDown: down)!.post(tap: .cghidEventTap)
      usleep(down ? 80_000 : 150_000)
    }
  }
case "click":
  activate()
  let point = CGPoint(x: bounds.minX + Double(args[2])!, y: bounds.minY + Double(args[3])!)
  for type in [CGEventType.leftMouseDown, .leftMouseUp] {
    CGEvent(mouseEventSource: source, mouseType: type, mouseCursorPosition: point, mouseButton: .left)!
      .post(tap: .cghidEventTap)
    usleep(120_000)
  }
default:
  exit(2)
}
EOF
    swiftc -suppress-warnings -o "$helper" "$source" || die "swiftc failed"
  fi
  echo "$helper"
}

mac_key_code() {
  case "$1" in
    F1) echo 122 ;; Return) echo 36 ;; BackSpace) echo 51 ;; Escape) echo 53 ;; Tab) echo 48 ;; space) echo 49 ;;
    Left) echo 123 ;; Right) echo 124 ;; Down) echo 125 ;; Up) echo 126 ;;
    a) echo 0 ;; s) echo 1 ;; d) echo 2 ;; f) echo 3 ;; h) echo 4 ;; g) echo 5 ;; z) echo 6 ;; x) echo 7 ;;
    c) echo 8 ;; v) echo 9 ;; b) echo 11 ;; q) echo 12 ;; w) echo 13 ;; e) echo 14 ;; r) echo 15 ;; y) echo 16 ;;
    t) echo 17 ;; o) echo 31 ;; u) echo 32 ;; i) echo 34 ;; p) echo 35 ;; l) echo 37 ;; j) echo 38 ;; k) echo 40 ;;
    n) echo 45 ;; m) echo 46 ;;
    *) die "no macOS key code for $1" ;;
  esac
}

mac() {
  local cmd="$1"
  shift
  local helper
  helper=$(mac_helper)
  case "$cmd" in
    key)
      local codes=() key
      for key in "$@"; do codes+=("$(mac_key_code "$key")"); done
      "$helper" key "${codes[@]}"
      ;;
    click) "$helper" click "$1" "$2" ;;
    shot) screencapture -o -x -l "$("$helper" winid)" "$1" ;;
    rect) "$helper" rect ;;
  esac
}

case "${1:-}" in
  key) (($# >= 2)) || die "key needs at least one key" ;;
  click) (($# == 3)) || die "click needs x and y" ;;
  shot) (($# == 2)) || die "shot needs an output file" ;;
  rect) ;;
  -h | --help | help)
    usage
    exit 0
    ;;
  *)
    usage >&2
    exit 2
    ;;
esac

if [[ "$(uname -s)" == Darwin ]]; then mac "$@"; else linux "$@"; fi
