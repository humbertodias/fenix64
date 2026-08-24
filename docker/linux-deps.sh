#!/usr/bin/env bash
# Build static SDL 1.2 and SDL_mixer (Ogg in-tree) into $FENIX_STATIC_PREFIX.
# Distro libSDL_mixer.a pulls FLAC/mikmod/fluidsynth; Ubuntu libSDL.a pulls caca.
# Static Fenix should only need X11 / ALSA / Pulse at runtime, not those .so files.
set -euo pipefail

PREFIX="${FENIX_STATIC_PREFIX:-/opt/fenix-static}"
SRC="${FENIX_STATIC_SRC:-/tmp/fenix-static-src}"
NPROC="$(nproc)"

mkdir -p "$PREFIX"/{bin,include,lib/pkgconfig} "$SRC"
export PATH="$PREFIX/bin:$PATH"
export PKG_CONFIG_PATH="$PREFIX/lib/pkgconfig${PKG_CONFIG_PATH:+:$PKG_CONFIG_PATH}"
export CPPFLAGS="-I$PREFIX/include ${CPPFLAGS:-}"
export LDFLAGS="-L$PREFIX/lib ${LDFLAGS:-}"

cd "$SRC"

fetch() {
  echo "Downloading $1"
  curl -fL --retry 5 --retry-all-errors -o "$2" "$1"
}

patch_config_guess() {
  if [ -f /tmp/config.guess ]; then
    [ -d "$1" ] || return 0
    cp /tmp/config.guess /tmp/config.sub "$1/"
    if [ -d "$1/build-scripts" ]; then
      cp /tmp/config.guess /tmp/config.sub "$1/build-scripts/"
    fi
  fi
}

fetch https://www.libsdl.org/release/SDL-1.2.15.tar.gz sdl.tar.gz \
  || fetch https://github.com/libsdl-org/SDL-1.2/releases/download/release-1.2.15/SDL-1.2.15.tar.gz sdl.tar.gz
tar xzf sdl.tar.gz
cd SDL-1.2.15
patch_config_guess .
# GCC 11 / current Xlib vs SDL 1.2.15 (Debian libsdl1.2 const patches).
sed -i \
  -e 's/register long \*data/const long *data/' \
  -e 's/_XRead32,(Display \*dpy,const long \*data/_XRead32,(Display *dpy,long *data/' \
  src/video/x11/SDL_x11sym.h
./configure --prefix="$PREFIX" --enable-static --disable-shared \
  --disable-nasm --disable-video-opengl --disable-video-caca \
  --disable-video-directfb --disable-arts --disable-esd --disable-nas \
  --enable-video-x11 --enable-alsa --enable-pulseaudio
make -j"$NPROC"
make install
cd ..

fetch https://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz libogg.tar.gz \
  || fetch https://github.com/xiph/ogg/releases/download/v1.3.5/libogg-1.3.5.tar.gz libogg.tar.gz
tar xzf libogg.tar.gz
cd libogg-1.3.5
patch_config_guess .
./configure --prefix="$PREFIX" --enable-static --disable-shared
make -j"$NPROC"
make install
cd ..

fetch https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz libvorbis.tar.gz \
  || fetch https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.gz libvorbis.tar.gz
tar xzf libvorbis.tar.gz
cd libvorbis-1.3.7
patch_config_guess .
./configure --prefix="$PREFIX" --enable-static --disable-shared \
  --with-ogg="$PREFIX" --disable-oggtest --disable-examples
make -j"$NPROC"
make install
cd ..

fetch https://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-1.2.12.tar.gz sdl_mixer.tar.gz
tar xzf sdl_mixer.tar.gz
cd SDL_mixer-1.2.12
patch_config_guess .
# libtool's test uses gcc -static; libvorbisfile.a also needs -lvorbis -logg.
ac_cv_lib_vorbisfile_ov_open_callbacks=yes \
LIBS="-lvorbisfile -lvorbis -logg ${LIBS:-}" \
./configure --prefix="$PREFIX" --enable-static --disable-shared \
  --disable-sdltest \
  --disable-music-mp3 --disable-music-mod \
  --enable-music-ogg --disable-music-ogg-shared --disable-music-flac \
  SDL_CONFIG="$PREFIX/bin/sdl-config"
grep -q -- '-lvorbisfile' Makefile || {
  echo "SDL_mixer configured without Ogg/Vorbis" >&2
  exit 1
}
mkdir -p build
make -j"$NPROC" build/libSDL_mixer.la
make install-hdrs install-lib
cd ..

test -f "$PREFIX/lib/libSDL.a"
test -f "$PREFIX/lib/libSDL_mixer.a"
test -f "$PREFIX/lib/libogg.a"
test -f "$PREFIX/lib/libvorbis.a"
test -f "$PREFIX/lib/libvorbisfile.a"
test -x "$PREFIX/bin/sdl-config"
# Shared objects here would win over the .a files at Fenix link time.
! find "$PREFIX/lib" -name '*.so*' | grep -q .
cd /
rm -rf "$SRC"

echo "static SDL prefix: $PREFIX"
"$PREFIX/bin/sdl-config" --static-libs
