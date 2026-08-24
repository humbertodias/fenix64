#!/usr/bin/env bash
# Bundle shared-library deps next to ELF binaries using linuxdeploy, then
# flatten usr/bin + usr/lib into DEST with DT_RUNPATH=$ORIGIN so the zip
# root matches Windows/macOS (./fxi next to libSDL*.so).
#
#   bash scripts/linux-bundle-so.sh dist/linux-shared ./fxc ./fxi ./map ./fpg
set -euo pipefail

DEST="${1:-}"
[[ -n "$DEST" && -d "$DEST" ]] || {
  echo "usage: $0 <directory-of-binaries> [bin ...]" >&2
  exit 1
}
DEST="$(cd "$DEST" && pwd)"
shift || true

LINUXDEPLOY="${LINUXDEPLOY:-/opt/linuxdeploy/AppRun}"
if [[ ! -x "$LINUXDEPLOY" ]]; then
  if command -v linuxdeploy >/dev/null 2>&1; then
    LINUXDEPLOY="$(command -v linuxdeploy)"
  else
    echo "linuxdeploy not found (expected $LINUXDEPLOY)." >&2
    exit 1
  fi
fi

PATCHELF="${PATCHELF:-}"
if [[ -z "$PATCHELF" ]]; then
  if [[ -x /opt/linuxdeploy/usr/bin/patchelf ]]; then
    PATCHELF=/opt/linuxdeploy/usr/bin/patchelf
  elif command -v patchelf >/dev/null 2>&1; then
    PATCHELF="$(command -v patchelf)"
  else
    echo "patchelf is required to flatten linuxdeploy's \$ORIGIN/../lib rpath." >&2
    exit 1
  fi
fi

bins=()
if [[ $# -gt 0 ]]; then
  bins=("$@")
else
  while IFS= read -r f; do
    bins+=("$f")
  done < <(find "$DEST" -maxdepth 1 -type f -perm -111 ! -name '*.so*')
fi

[[ ${#bins[@]} -gt 0 ]] || {
  echo "no binaries in $DEST" >&2
  exit 1
}

APPDIR="$(mktemp -d "${TMPDIR:-/tmp}/fenix-appdir.XXXXXX")"
cleanup() { rm -rf "$APPDIR"; }
trap cleanup EXIT

ld_args=(--appdir "$APPDIR")
for bin in "${bins[@]}"; do
  [[ -f "$bin" ]] || continue
  ld_args+=(-e "$bin")
done

# linuxdeploy is an AppImage; Docker has no FUSE.
export APPIMAGE_EXTRACT_AND_RUN=1
# Do not strip; Fenix CI artifacts keep debug info on the executables.
export NO_STRIP=1
"$LINUXDEPLOY" "${ld_args[@]}"

cp -a "$APPDIR"/usr/bin/. "$DEST/"
if [[ -d "$APPDIR/usr/lib" ]]; then
  find "$APPDIR/usr/lib" -type f -name '*.so*' -print0 \
    | while IFS= read -r -d '' lib; do
        cp -a "$lib" "$DEST/$(basename "$lib")"
      done
  find "$APPDIR/usr/lib" -type l -name '*.so*' -print0 \
    | while IFS= read -r -d '' lib; do
        target="$(basename "$(readlink -f "$lib")")"
        ln -sfn "$target" "$DEST/$(basename "$lib")"
      done
fi

while IFS= read -r f; do
  [[ -f "$f" && ! -L "$f" ]] || continue
  file -b "$f" | grep -q 'ELF' || continue
  "$PATCHELF" --set-rpath '$ORIGIN' "$f"
done < <(find "$DEST" -maxdepth 1 -type f)

# linuxdeploy's AppImage exclude list skips libz; libpng still needs it.
if [[ ! -e "$DEST/libz.so.1" ]]; then
  for cand in /lib/x86_64-linux-gnu/libz.so.1 /usr/lib/x86_64-linux-gnu/libz.so.1; do
    [[ -e "$cand" ]] || continue
    real="$(readlink -f "$cand")"
    cp -a "$real" "$DEST/$(basename "$real")"
    ln -sfn "$(basename "$real")" "$DEST/libz.so.1"
    "$PATCHELF" --set-rpath '$ORIGIN' "$DEST/$(basename "$real")"
    break
  done
fi

has_mixer=0
has_sdl=0
for f in "$DEST"/libSDL_mixer*.so* "$DEST"/libSDL-1.2*.so*; do
  [[ -e "$f" ]] || continue
  case "$(basename "$f")" in
    libSDL_mixer*) has_mixer=1 ;;
    libSDL-1.2*) has_sdl=1 ;;
  esac
done
if [[ "$has_sdl" -eq 0 ]]; then
  echo "bundled linux-shared without libSDL-1.2; fxc/fxi will fail at runtime" >&2
  ls -l "$DEST" >&2
  exit 1
fi
if [[ "$has_mixer" -eq 0 ]]; then
  echo "bundled linux-shared without libSDL_mixer; fxi will fail at runtime" >&2
  ls -l "$DEST" >&2
  exit 1
fi
