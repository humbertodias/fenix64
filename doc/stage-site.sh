#!/usr/bin/env bash
# Stage dist/pages from project docs + Doxygen (see doc/pages.yml).
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
DST="${1:-$ROOT/dist/pages}"

bash "$ROOT/doc/generate.sh"

mkdir -p "$DST/docs" "$DST/formats"
cp "$ROOT/doc/index.html" "$DST/"
cp "$ROOT/doc/site.css" "$DST/"
cp "$ROOT/readme.html" "$DST/readme.html"
cp "$ROOT/info.html" "$DST/info.html"
# Historical pages expect fenix.css next to the HTML.
cp "$ROOT/doc/site.css" "$DST/fenix.css"
cp "$ROOT/doc/formats.html" "$DST/formats/index.html"
cp "$ROOT/doc/site.css" "$DST/formats/site.css"
cp "$ROOT/debian/doc/"*.txt "$DST/formats/"
cp -a "$ROOT/doc/html/." "$DST/docs/"
touch "$DST/.nojekyll"

test -s "$DST/index.html"
test -s "$DST/readme.html"
test -s "$DST/info.html"
test -s "$DST/docs/index.html"

echo "Pages site staged at: $DST ($(find "$DST" -type f | wc -l) files)"
