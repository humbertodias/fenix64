#!/usr/bin/env bash
# Build zlib, libpng, giflib, SDL 1.2, libogg, libvorbis, and SDL_mixer for MinGW-w64 into $MINGW_PREFIX.
set -euo pipefail

HOST="${HOST:-x86_64-w64-mingw32}"
MINGW_PREFIX="${MINGW_PREFIX:-/opt/mingw}"
SRC="${MINGW_SRC:-/tmp/mingw-src}"
# SDL 1.2.15's config.guess (2009) cannot detect aarch64; --build is required
# when the Docker host is Apple Silicon. Do not export CC=mingw before configure.
BUILD="$(gcc -dumpmachine)"

mkdir -p "$MINGW_PREFIX"/{bin,include,lib/pkgconfig} "$SRC"
export PATH="$MINGW_PREFIX/bin:$PATH"
export PKG_CONFIG_LIBDIR="$MINGW_PREFIX/lib/pkgconfig"
export CPPFLAGS="-I$MINGW_PREFIX/include"
export LDFLAGS="-L$MINGW_PREFIX/lib"

cd "$SRC"

fetch() {
  echo "Downloading $1"
  curl -fL --retry 5 --retry-all-errors -o "$2" "$1"
}

# Both archives and DLLs so the same image can link static or shared Fenix.
SHARED_FLAG="--enable-shared --enable-static"

fetch https://github.com/madler/zlib/archive/refs/tags/v1.3.1.tar.gz zlib.tar.gz \
  || fetch https://zlib.net/fossils/zlib-1.3.1.tar.gz zlib.tar.gz
tar xzf zlib.tar.gz
make -C zlib-1.3.1 -f win32/Makefile.gcc PREFIX="${HOST}-" \
  BINARY_PATH="$MINGW_PREFIX/bin" \
  INCLUDE_PATH="$MINGW_PREFIX/include" \
  LIBRARY_PATH="$MINGW_PREFIX/lib" \
  SHARED_MODE=1 install

fetch https://github.com/pnggroup/libpng/archive/refs/tags/v1.6.43.tar.gz libpng.tar.gz
tar xzf libpng.tar.gz
cd libpng-1.6.43
./configure --build="$BUILD" --host="$HOST" --prefix="$MINGW_PREFIX" $SHARED_FLAG
make -j"$(nproc)"
make install
cd ..

fetch https://sourceforge.net/projects/giflib/files/giflib-5.2.2.tar.gz/download giflib.tar.gz
tar xzf giflib.tar.gz
make -C giflib-5.2.2 libgif.a CC="${HOST}-gcc" AR="${HOST}-ar"
cp giflib-5.2.2/libgif.a "$MINGW_PREFIX/lib/"
cp giflib-5.2.2/gif_lib.h "$MINGW_PREFIX/include/"

fetch https://www.libsdl.org/release/SDL-1.2.15.tar.gz sdl.tar.gz \
  || fetch https://github.com/libsdl-org/SDL-1.2/releases/download/release-1.2.15/SDL-1.2.15.tar.gz sdl.tar.gz
tar xzf sdl.tar.gz
cd SDL-1.2.15
if [ -f /tmp/config.guess ]; then
  cp /tmp/config.guess /tmp/config.sub build-scripts/
fi
if [ ! -f configure ]; then
  apt-get update
  apt-get install -y autoconf automake libtool
  ./autogen.sh
fi
./configure --build="$BUILD" --host="$HOST" --prefix="$MINGW_PREFIX" $SHARED_FLAG \
  --disable-video-x11 --disable-video-opengl --disable-nasm \
  --disable-directx --disable-video-directx --enable-video-windib
make -j"$(nproc)"
make install
cd ..

# HFF and other 0.84 games load .ogg music through SDL_mixer.
fetch https://downloads.xiph.org/releases/ogg/libogg-1.3.5.tar.gz libogg.tar.gz \
  || fetch https://github.com/xiph/ogg/releases/download/v1.3.5/libogg-1.3.5.tar.gz libogg.tar.gz
tar xzf libogg.tar.gz
cd libogg-1.3.5
if [ -f /tmp/config.guess ]; then
  cp /tmp/config.guess /tmp/config.sub .
fi
./configure --build="$BUILD" --host="$HOST" --prefix="$MINGW_PREFIX" $SHARED_FLAG
make -j"$(nproc)"
make install
cd ..

fetch https://downloads.xiph.org/releases/vorbis/libvorbis-1.3.7.tar.gz libvorbis.tar.gz \
  || fetch https://github.com/xiph/vorbis/releases/download/v1.3.7/libvorbis-1.3.7.tar.gz libvorbis.tar.gz
tar xzf libvorbis.tar.gz
cd libvorbis-1.3.7
if [ -f /tmp/config.guess ]; then
  cp /tmp/config.guess /tmp/config.sub .
fi
./configure --build="$BUILD" --host="$HOST" --prefix="$MINGW_PREFIX" $SHARED_FLAG \
  --with-ogg="$MINGW_PREFIX" --disable-oggtest --disable-examples
make -j"$(nproc)"
make install
cd ..

fetch https://www.libsdl.org/projects/SDL_mixer/release/SDL_mixer-1.2.12.tar.gz sdl_mixer.tar.gz
tar xzf sdl_mixer.tar.gz
cd SDL_mixer-1.2.12
if [ -f /tmp/config.guess ]; then
  cp /tmp/config.guess /tmp/config.sub .
  mkdir -p build-scripts
  cp /tmp/config.guess /tmp/config.sub build-scripts/
fi
./configure --build="$BUILD" --host="$HOST" --prefix="$MINGW_PREFIX" $SHARED_FLAG \
  --disable-sdltest \
  --disable-music-mp3 --disable-music-mod \
  --enable-music-ogg --disable-music-ogg-shared --disable-music-flac \
  SDL_CONFIG="$MINGW_PREFIX/bin/sdl-config"
grep -q -- '-lvorbisfile' Makefile || {
  echo "SDL_mixer configured without Ogg/Vorbis" >&2
  exit 1
}
# libSDL_mixer.a does not contain vorbis objects; Fenix links
# -lvorbisfile -lvorbis -logg (see configure.in).
# playwave/playmus fail to link against SDL.dll (undefined WinMain).
# 1.2.12 uses a `build` directory target, not `build/.created`.
mkdir -p build
make -j"$(nproc)" build/libSDL_mixer.la
make install-hdrs install-lib
cd ..

test -f "$MINGW_PREFIX/lib/libSDL.a"
test -f "$MINGW_PREFIX/lib/libSDL_mixer.a"
test -x "$MINGW_PREFIX/bin/sdl-config"
find "$MINGW_PREFIX" -name 'SDL.dll' -o -name 'libSDL.dll' | grep -q .
find "$MINGW_PREFIX" \( -name 'SDL_mixer.dll' -o -name 'libSDL_mixer*.dll' \) | grep -q .
rm -rf "$SRC"
