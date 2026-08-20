#!/usr/bin/env bash
# Copy non-system dylibs next to Mach-O binaries and rewrite install names
# to @executable_path so the tree runs without Homebrew.
#
# Homebrew's sdl12-compat (and often sdl2-compat) dlopen the next SDL at
# runtime; those libraries do not appear in otool -L. Follow LC_RPATH from
# the original keg so SDL2/SDL3 land next to the binaries (@loader_path).
#
#   bash scripts/macos-bundle-dylibs.sh dist/macos-static
set -euo pipefail

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-bundle-dylibs.sh is only for macOS." >&2
  exit 1
fi

DEST="${1:-}"
[[ -n "$DEST" && -d "$DEST" ]] || {
  echo "usage: $0 <directory-of-binaries>" >&2
  exit 1
}
DEST="$(cd "$DEST" && pwd)"

canon() {
  python3 -c 'import os,sys; print(os.path.realpath(sys.argv[1]))' "$1"
}

rpaths_of() {
  otool -l "$1" 2>/dev/null | awk '/cmd LC_RPATH/{c=1} c && /path /{print $2; c=0}'
}

should_bundle() {
  case "$1" in
    /System/*|/usr/lib/*|/Library/Apple/*|@executable_path/*|@loader_path/*|@rpath/*) return 1 ;;
    /opt/homebrew/*|/usr/local/*|/opt/local/*|*/deps/local/*) return 0 ;;
    *) return 1 ;;
  esac
}

load_dylibs() {
  otool -L "$1" 2>/dev/null | awk 'NR > 1 { print $1 }'
}

is_macho() {
  local f="$1"
  [[ -f "$f" && ! -L "$f" ]] || return 1
  file -b "$f" | grep -q 'Mach-O'
}

rewrite_rpaths() {
  local lib="$1" rp
  while IFS= read -r rp; do
    [[ -n "$rp" ]] || continue
    [[ "$rp" == "@loader_path" || "$rp" == "@executable_path" ]] && continue
    install_name_tool -delete_rpath "$rp" "$lib" 2>/dev/null || true
  done < <(rpaths_of "$lib")
  install_name_tool -add_rpath "@loader_path" "$lib" 2>/dev/null || true
  install_name_tool -add_rpath "@executable_path" "$lib" 2>/dev/null || true
}

# Copy a keg dylib (and its basename aliases) into DEST.
copy_keg_dylib() {
  local src="$1" real realbase dest_lib base
  [[ -f "$src" ]] || return 0
  real="$(canon "$src")"
  realbase="$(basename "$real")"
  dest_lib="$DEST/$realbase"
  if [[ ! -e "$dest_lib" ]]; then
    cp -L "$real" "$dest_lib"
    chmod u+w "$dest_lib"
    install_name_tool -id "@executable_path/$realbase" "$dest_lib"
    rewrite_rpaths "$dest_lib"
    seed_rpath_dylibs "$real"
  fi
  base="$(basename "$src")"
  if [[ "$base" != "$realbase" && ! -e "$DEST/$base" ]]; then
    ln -sf "$realbase" "$DEST/$base"
  fi
}

seed_from_dir() {
  local dir="$1" f
  [[ -d "$dir" ]] || return 0
  dir="$(canon "$dir" 2>/dev/null)" || return 0
  for f in "$dir"/lib*.dylib; do
    [[ -e "$f" ]] || continue
    copy_keg_dylib "$f"
  done
}

# sdl12-compat / sdl2-compat look for the next SDL via LC_RPATH (Cellar layout).
seed_rpath_dylibs() {
  local origin="$1" origin_dir rpath resolved keg
  origin="$(canon "$origin")"
  origin_dir="$(dirname "$origin")"
  while IFS= read -r rpath; do
    [[ -n "$rpath" ]] || continue
    resolved=""
    case "$rpath" in
      @loader_path/*) resolved="$origin_dir/${rpath#@loader_path/}" ;;
      @loader_path) resolved="$origin_dir" ;;
      /*) resolved="$rpath" ;;
      *) continue ;;
    esac
    seed_from_dir "$resolved"
    # Copied dylibs keep a Cellar-relative rpath; also try Homebrew prefixes.
    if [[ "$rpath" == *"/opt/"* ]]; then
      keg="${rpath##*/opt/}"
      seed_from_dir "/opt/homebrew/opt/${keg}"
      seed_from_dir "/usr/local/opt/${keg}"
    fi
  done < <(rpaths_of "$origin")
}

bundle_one() {
  local bin="$1" dep base dest_lib
  while IFS= read -r dep; do
    [[ -n "$dep" ]] || continue
    should_bundle "$dep" || continue
    [[ -f "$dep" ]] || continue
    base="$(basename "$dep")"
    dest_lib="$DEST/$base"
    if [[ ! -e "$dest_lib" ]]; then
      cp -L "$dep" "$dest_lib"
      chmod u+w "$dest_lib"
      install_name_tool -id "@executable_path/$base" "$dest_lib"
      rewrite_rpaths "$dest_lib"
      seed_rpath_dylibs "$dep"
    fi
    install_name_tool -change "$dep" "@executable_path/$base" "$bin"
  done < <(load_dylibs "$bin")
}

has_sdl1() {
  local f
  for f in "$DEST"/libSDL-1.2*.dylib; do
    [[ -e "$f" ]] && return 0
  done
  return 1
}

has_sdl2() {
  local f
  for f in "$DEST"/libSDL2-2.0*.dylib "$DEST"/libSDL2.dylib; do
    [[ -e "$f" ]] && return 0
  done
  return 1
}

# If LC_RPATH did not yield SDL2 (older bottles), look in well-known prefixes.
ensure_sdl2() {
  has_sdl1 || return 0
  has_sdl2 && return 0
  local d candidate
  for d in \
      /opt/homebrew/opt/sdl2/lib \
      /opt/homebrew/opt/sdl2-compat/lib \
      /usr/local/opt/sdl2/lib \
      /usr/local/opt/sdl2-compat/lib; do
    for candidate in "$d"/libSDL2-2.0.0.dylib "$d"/libSDL2-2.0.dylib; do
      if [[ -f "$candidate" ]]; then
        copy_keg_dylib "$candidate"
        return 0
      fi
    done
  done
  echo "sdl12-compat needs libSDL2 next to the binary (brew install sdl2)." >&2
  exit 1
}

pass=0
while true; do
  pass=$((pass + 1))
  [[ "$pass" -le 20 ]] || {
    echo "dylib bundling did not converge" >&2
    exit 1
  }
  before="$(find "$DEST" -maxdepth 1 \( -type f -o -type l \) -print | wc -l | tr -d ' ')"
  for f in "$DEST"/*.dylib; do
    is_macho "$f" || continue
    seed_rpath_dylibs "$f"
  done
  ensure_sdl2
  for f in "$DEST"/*; do
    is_macho "$f" || continue
    chmod u+w "$f"
    bundle_one "$f"
  done
  after="$(find "$DEST" -maxdepth 1 \( -type f -o -type l \) -print | wc -l | tr -d ' ')"
  leftover=0
  for f in "$DEST"/*; do
    is_macho "$f" || continue
    while IFS= read -r dep; do
      should_bundle "$dep" && leftover=1
    done < <(load_dylibs "$f")
  done
  [[ "$leftover" -eq 0 && "$after" -eq "$before" ]] && break
done

if has_sdl1 && ! has_sdl2; then
  echo "bundled SDL 1.2 without SDL2; sdl12-compat will fail at runtime" >&2
  exit 1
fi

if command -v codesign >/dev/null 2>&1; then
  for f in "$DEST"/*; do
    is_macho "$f" || continue
    codesign --force -s - "$f" >/dev/null 2>&1 || true
  done
fi

echo "Bundled dylibs in $DEST:"
ls -1 "$DEST"/*.dylib 2>/dev/null || echo "(none)"
