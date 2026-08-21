#!/usr/bin/env bash
# Native macOS build (Homebrew SDL 1.2 + SDL_mixer from source).
#
#   bash scripts/macos-build.sh
#   bash scripts/macos-build.sh static
#   bash scripts/macos-build.sh shared
#
# Skip already-installed pieces:
#   SKIP_BREW=1 SKIP_MIXER=1 bash scripts/macos-build.sh
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

USAGE="usage: $0 [static|shared]"

if [[ "$(uname -s)" != "Darwin" ]]; then
  echo "macos-build.sh is only for macOS (Apple SDK)." >&2
  exit 1
fi

LINKAGE="${1:-static}"
case "${LINKAGE}" in
  static|shared) ;;
  *)
    echo "${USAGE}" >&2
    exit 1
    ;;
esac

if ! command -v brew >/dev/null 2>&1; then
  echo "Homebrew is required (https://brew.sh)." >&2
  exit 1
fi

BREW_PREFIX="$(brew --prefix)"
export PATH="${BREW_PREFIX}/bin:${PATH}"
NCPU="$(sysctl -n hw.ncpu)"
MIXER_PREFIX="${ROOT}/deps/local"
MIXER_VERSION="${MIXER_VERSION:-1.2.12}"
STAGE="${ROOT}/dist/macos-${LINKAGE}"

if [[ "${SKIP_BREW:-}" != "1" ]]; then
  brew install sdl12-compat sdl2 sdl3 libpng giflib pkg-config libogg libvorbis
fi

command -v pkg-config >/dev/null 2>&1 || {
  echo "pkg-config is required (brew install pkg-config)." >&2
  exit 1
}
pkg-config --exists sdl || {
  echo "SDL 1.2 is missing (brew install sdl12-compat)." >&2
  exit 1
}

mixer_dylib="${MIXER_PREFIX}/lib/libSDL_mixer-1.2.0.dylib"
mixer_needs_ogg=0
if [[ -f "${mixer_dylib}" ]] && ! otool -L "${mixer_dylib}" | grep -q vorbis; then
  mixer_needs_ogg=1
fi
if [[ "${SKIP_MIXER:-}" != "1" && ( ! -f "${MIXER_PREFIX}/lib/pkgconfig/SDL_mixer.pc" || "${mixer_needs_ogg}" = 1 ) ]]; then
  mkdir -p "${ROOT}/deps"
  (
    cd "${ROOT}/deps"
    TARBALL="SDL_mixer-${MIXER_VERSION}.tar.gz"
    if [[ ! -f "${TARBALL}" ]]; then
      curl -LO "https://www.libsdl.org/projects/SDL_mixer/release/${TARBALL}"
    fi
    rm -rf "SDL_mixer-${MIXER_VERSION}"
    tar xzf "${TARBALL}"
    cd "SDL_mixer-${MIXER_VERSION}"
    # pkg-config sdl only adds .../include/SDL, so vorbis/vorbisfile.h is missed
    # unless Homebrew's include dir is on the path. HFF and other 0.84 games
    # load .ogg music through SDL_mixer.
    ./configure --prefix="${MIXER_PREFIX}" \
        --disable-sdltest --disable-music-native-midi \
        --disable-music-fluidsynth-midi --disable-music-flac \
        --enable-music-ogg --disable-music-ogg-shared \
        --enable-shared --enable-static \
        CPPFLAGS="$(pkg-config --cflags sdl) -I${BREW_PREFIX}/include" \
        CFLAGS="-g -O2 -Wno-incompatible-function-pointer-types" \
        LDFLAGS="$(pkg-config --libs-only-L sdl) -L${BREW_PREFIX}/lib" \
        LIBS="$(pkg-config --libs-only-l sdl)"
    make -j"${NCPU}"
    make install
  )
fi

[[ -f "${MIXER_PREFIX}/lib/pkgconfig/SDL_mixer.pc" ]] || {
  echo "SDL_mixer is missing under ${MIXER_PREFIX} (unset SKIP_MIXER)." >&2
  exit 1
}

# Autotools records an absolute install name; rewrite so linkage survives a moved tree.
if [[ -f "${MIXER_PREFIX}/lib/libSDL_mixer-1.2.0.dylib" ]]; then
  chmod u+w "${MIXER_PREFIX}/lib/libSDL_mixer-1.2.0.dylib"
  install_name_tool -id "@rpath/libSDL_mixer-1.2.0.dylib" \
      "${MIXER_PREFIX}/lib/libSDL_mixer-1.2.0.dylib"
fi

export PKG_CONFIG_PATH="${MIXER_PREFIX}/lib/pkgconfig:${BREW_PREFIX}/lib/pkgconfig${PKG_CONFIG_PATH:+:${PKG_CONFIG_PATH}}"
export CPPFLAGS="-I${MIXER_PREFIX}/include ${CPPFLAGS:-}"
export LDFLAGS="-L${MIXER_PREFIX}/lib -Wl,-rpath,@executable_path ${LDFLAGS:-}"
if [[ "${LINKAGE}" == "static" ]]; then
  export LDFLAGS="${LDFLAGS} -Wl,-search_paths_first"
fi

echo "linkage: ${LINKAGE}"
echo "stage: ${STAGE}"

rm -f config.cache
if [[ -f Makefile ]]; then
  make distclean || true
fi

./configure --enable-fpg --enable-map
make -j"${NCPU}"

rm -rf "${STAGE}"
mkdir -p "${STAGE}"
BINS=(fxc/src/fxc fxi/src/fxi map/map fpg/fpg)
cp "${BINS[@]}" "${STAGE}/"
bash "${ROOT}/scripts/macos-bundle-dylibs.sh" "${STAGE}"

otool -L "${STAGE}/fxc" "${STAGE}/fxi"
if otool -L "${STAGE}"/* | grep -E '/opt/homebrew/|/usr/local/opt/|/deps/local/'; then
  echo "${LINKAGE} artifact still references Homebrew or deps/local" >&2
  exit 1
fi
file "${STAGE}"/*
HELP_OUT="$("${STAGE}/fxc" -h 2>&1 || true)"
printf '%s\n' "${HELP_OUT}"
if printf '%s\n' "${HELP_OUT}" | grep -q 'Failed loading SDL2'; then
  echo "fxc could not load SDL2; expected libSDL2 next to the binary" >&2
  ls -l "${STAGE}" >&2
  exit 1
fi
echo "ok: ${STAGE}"
