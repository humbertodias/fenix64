#!/usr/bin/env bash
# Build Fenix inside a toolchain image (autoconf/make + SDL 1.2).
#
#   bash scripts/docker-build.sh
#   bash scripts/docker-build.sh linux shared
#   bash scripts/docker-build.sh windows
#   bash scripts/docker-build.sh linux shell
set -euo pipefail

ROOT="$(cd "$(dirname "$0")/.." && pwd)"
cd "${ROOT}"

USAGE="usage: $0 linux|windows [static|shared|shell]"

if ! command -v docker >/dev/null 2>&1; then
  echo "Docker is required (https://docs.docker.com/get-docker/)." >&2
  exit 1
fi

PLATFORM="${1:-linux}"
SECOND="${2:-}"
if [[ "${PLATFORM}" == *-* && -z "${SECOND}" ]]; then
  SECOND="${PLATFORM#*-}"
  PLATFORM="${PLATFORM%%-*}"
fi
SECOND="${SECOND:-static}"
case "${PLATFORM}" in
  linux|windows) ;;
  *)
    echo "${USAGE}" >&2
    exit 1
    ;;
esac

IMAGE="fenix-${PLATFORM}"
# Always amd64 so a Mac/ARM Docker host matches GitHub Actions (x86_64 Linux / MinGW).
DOCKER_PLATFORM="${DOCKER_PLATFORM:-linux/amd64}"

if [[ "${SKIP_DOCKER_BUILD:-}" != "1" ]]; then
  docker build --platform "${DOCKER_PLATFORM}" -t "${IMAGE}" -f "docker/Dockerfile.${PLATFORM}" docker/
fi

if [[ "${SECOND}" == "shell" ]]; then
  exec docker run --platform "${DOCKER_PLATFORM}" --rm -it \
    -v "${ROOT}:/src" \
    -w /src \
    -e HOME=/tmp \
    -e TERM="${TERM:-xterm}" \
    "${IMAGE}" bash
fi

LINKAGE="${SECOND}"
case "${LINKAGE}" in
  static|shared) ;;
  *)
    echo "${USAGE}" >&2
    exit 1
    ;;
esac

echo "image: ${IMAGE}"
echo "platform: ${DOCKER_PLATFORM}"
echo "linkage: ${LINKAGE}"

docker run --platform "${DOCKER_PLATFORM}" --rm \
  -u "$(id -u):$(id -g)" \
  -v "${ROOT}:/src" \
  -w /src \
  -e HOME=/tmp \
  -e TERM=dumb \
  -e PLATFORM="${PLATFORM}" \
  -e LINKAGE="${LINKAGE}" \
  "${IMAGE}" \
  bash -c 'set -euo pipefail
    rm -f config.cache
    if [[ -f Makefile ]]; then
      make distclean || true
    fi
    # distclean uses the leftover Makefile. A macOS Makefile often fails
    # inside this image, so .o files stay and MinGW ld rejects them
    # ("file format not recognized", e.g. fxc/src/messages.o).
    # Do not combine -prune with -delete: -delete implies -depth and find
    # treats that as an error (GNU find 4.8+).
    find . -type f \( -name "*.o" -o -name "*.obj" -o -name "*.lo" -o -name "*.exe" \) \
      ! -path "./.git/*" ! -path "./deps/*" ! -path "./dist/*" -delete
    find . -type d \( -name .deps -o -name .libs \) \
      ! -path "./.git/*" ! -path "./deps/*" ! -path "./dist/*" -print0 | xargs -0r rm -rf

    if [[ "${PLATFORM}" == "windows" ]]; then
      HOST="${HOST:-x86_64-w64-mingw32}"
      MINGW_PREFIX="${MINGW_PREFIX:-/opt/mingw}"
      export PATH="${MINGW_PREFIX}/bin:${PATH}"
      export MINGW_PREFIX HOST
      export PKG_CONFIG_LIBDIR="${MINGW_PREFIX}/lib/pkgconfig"
      export CPPFLAGS="-I${MINGW_PREFIX}/include"
      export LDFLAGS="-L${MINGW_PREFIX}/lib"
      if [[ "${LINKAGE}" == "static" ]]; then
        export LDFLAGS="${LDFLAGS} -static"
      fi
      export SDL_CONFIG="${MINGW_PREFIX}/bin/sdl-config"
      ./configure --build="$(gcc -dumpmachine)" --host="${HOST}" --enable-fpg --enable-map --disable-sdltest
      STAGE="/src/dist/windows-${LINKAGE}"
      BINS=(fxc/src/fxc.exe fxi/src/fxi.exe map/map.exe fpg/fpg.exe)
    else
      pkg-config --exists libpng zlib || {
        echo "libpng/zlib missing in the image." >&2
        pkg-config --list-all | sort >&2 || true
        dpkg -l "libpng*" "zlib*" >&2 || true
        exit 1
      }
      if [[ "${LINKAGE}" == "static" ]]; then
        export PKG_CONFIG="pkg-config --static"
        export LDFLAGS="${LDFLAGS:-} -static-libgcc"
      fi
      export CPPFLAGS="$(pkg-config --cflags libpng zlib) ${CPPFLAGS:-}"
      export LIBS="$(pkg-config --libs libpng zlib) ${LIBS:-}"
      # QEMU amd64 on Apple Silicon can flake Autoconf AC_CHECK_LIB link tests.
      export ac_cv_lib_png_png_read_info=yes
      export ac_cv_header_png_h=yes
      export ac_cv_lib_z_gzsetparams=yes
      export ac_cv_header_zlib_h=yes
      ./configure --enable-fpg --enable-map
      STAGE="/src/dist/linux-${LINKAGE}"
      BINS=(fxc/src/fxc fxi/src/fxi map/map fpg/fpg)
    fi

    make -j"$(nproc)"
    rm -rf "${STAGE}"
    mkdir -p "${STAGE}"
    cp "${BINS[@]}" "${STAGE}/"
    if [[ "${PLATFORM}" == "windows" && "${LINKAGE}" == "shared" ]]; then
      cp "${MINGW_PREFIX}/bin/"*.dll "${STAGE}/" 2>/dev/null || true
    fi
    for bin in "${BINS[@]}"; do
      test -e "${STAGE}/$(basename "${bin}")"
    done
    file "${STAGE}"/*
    if [[ "${PLATFORM}" == "linux" ]]; then
      "${STAGE}/fxc" -h || true
    fi
  '
