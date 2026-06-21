#!/usr/bin/env bash
set -euo pipefail

SOURCE_DIR="${1:?source directory (repo root) required}"
BUILD_DIR="${2:?build directory required}"

EXAMPLES_DIR="$SOURCE_DIR/js/examples"
DEST="$BUILD_DIR/vitadeck-data/installed-deck-apps"

if [[ ! -d "$EXAMPLES_DIR" ]]; then
  echo "Examples directory missing: $EXAMPLES_DIR" >&2
  exit 1
fi

mkdir -p "$DEST"

staged=0
for example_dir in "$EXAMPLES_DIR"/*/; do
  [[ -d "$example_dir" ]] || continue
  name=$(basename "$example_dir")
  if [[ "$name" == "node_modules" ]] || [[ ! -f "$example_dir/vitadeck.config.json" ]]; then
    continue
  fi
  vdapp_src=""
  for candidate in "$example_dir/dist/"*.vdapp; do
    if [[ -d "$candidate" ]]; then
      vdapp_src="$candidate"
      break
    fi
  done
  if [[ -z "$vdapp_src" ]]; then
    echo "Skipping $name: no dist/*.vdapp (run pnpm --dir js build first)" >&2
    continue
  fi
  pkg_name=$(basename "$vdapp_src")
  rm -rf "$DEST/$pkg_name"
  cp -R "$vdapp_src" "$DEST/$pkg_name"
  echo "Staged $pkg_name"
  staged=$((staged + 1))
done

if [[ "$staged" -eq 0 ]]; then
  echo "No example Deck Apps staged. Run pnpm --dir js build first." >&2
  exit 1
fi

echo "Staged $staged example Deck App(s) to $DEST"
