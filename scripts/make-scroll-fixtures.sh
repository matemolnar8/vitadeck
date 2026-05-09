#!/usr/bin/env sh
set -eu

COUNT="${2:-20}"
DATA_ROOT="${VITADECK_DATA_ROOT:-vitadeck-data/installed-deck-apps}"
FALLBACK_DATA_ROOT="out/vitadeck-data/installed-deck-apps"
PREFIX="scroll-test-"

if [ "${1:-}" = "clean" ]; then
  rm -rf "$DATA_ROOT"/"$PREFIX"*.vdapp
  echo "Removed fixtures from $DATA_ROOT"
  exit 0
fi

if [ ! -d "$DATA_ROOT" ]; then
  if [ -d "$FALLBACK_DATA_ROOT" ]; then
    DATA_ROOT="$FALLBACK_DATA_ROOT"
  else
    echo "Missing deck app library: $DATA_ROOT" >&2
    echo "Also checked fallback: $FALLBACK_DATA_ROOT" >&2
    exit 1
  fi
fi

BASE="$(ls "$DATA_ROOT" | rg '\.vdapp$' | rg -v "^$PREFIX" | sed -n '1p')"
if [ -z "$BASE" ]; then
  echo "No base .vdapp package found in $DATA_ROOT" >&2
  exit 1
fi

i=1
while [ "$i" -le "$COUNT" ]; do
  NEW="$PREFIX$i.vdapp"
  DEST="$DATA_ROOT/$NEW"
  rm -rf "$DEST"
  cp -R "$DATA_ROOT/$BASE" "$DEST"

  python3 - "$DEST/manifest.json" "$i" <<'PY'
import json, pathlib, sys
p = pathlib.Path(sys.argv[1])
i = int(sys.argv[2])
m = json.loads(p.read_text())
m["name"] = f"Scroll Test {i}"
m["version"] = f"1.0.{i}"
p.write_text(json.dumps(m, indent=2) + "\n")
PY

  i=$((i + 1))
done

echo "Created $COUNT fixtures in $DATA_ROOT"
