# Building Fenix

## Docker (Linux and Windows)

No local compiler or MinGW. Only [Docker](https://www.docker.com/get-started/).

```shell
bash scripts/docker-build.sh linux
bash scripts/docker-build.sh linux shared
bash scripts/docker-build.sh windows
bash scripts/docker-build.sh windows shared
```

| Command | Image | Output |
|---------|-------|--------|
| `linux` / `linux shared` | `docker/Dockerfile.linux` | `dist/linux-{static,shared}/` |
| `windows` / `windows shared` | `docker/Dockerfile.windows` | `dist/windows-{static,shared}/` |

Default linkage is **static**. Images are toolchains only (Ubuntu 22.04, SDL 1.2 with Ogg/Vorbis in SDL_mixer). Linux uses the distro mixer; Windows cross-builds libogg, libvorbis, and SDL_mixer 1.2 in `docker/mingw-deps.sh`. `scripts/docker-build.sh` builds the image, then `docker run` with the repo mounted at `/src` and autoconf/`make`. GitHub Actions uses the same Dockerfiles (`docker/build-push-action` + the same wrapper). Linux artifacts are **x86_64** (`--platform linux/amd64`).

```shell
bash scripts/docker-build.sh linux shell
bash scripts/docker-build.sh windows shell
```

## macOS

Apple SDK cannot run in Linux containers. On a Mac:

```shell
bash scripts/macos-build.sh
bash scripts/macos-build.sh shared
```

| Command | Output |
|---------|--------|
| `static` (default) | `dist/macos-static/` |
| `shared` | `dist/macos-shared/` |

Needs Homebrew (`sdl12-compat`, `sdl2`, `sdl3`, `libpng`, `giflib`, `pkg-config`, `libogg`, `libvorbis`). Current Homebrew `sdl12-compat` dlopens SDL2, and `sdl2` is often `sdl2-compat` which dlopens SDL3 — those dylibs are not in `otool -L`, so the bundler follows `LC_RPATH` to copy them. The script builds SDL_mixer 1.2.12 into `deps/local` with Ogg linked in (`SKIP_BREW=1` / `SKIP_MIXER=1` skip those steps). Static uses `LDFLAGS=-Wl,-search_paths_first`. After linking, `scripts/macos-bundle-dylibs.sh` copies SDL (and other Homebrew dylibs) next to the binaries with `@executable_path`, so the zip runs without Homebrew. GitHub Actions calls the same wrapper.

## Native autoconf

A C compiler, Autotools-generated `configure`, and SDL 1.2 / SDL_mixer / libpng / zlib / gif:

```shell
./configure --enable-fpg --enable-map
make
```

| Tool | Path |
|------|------|
| fxc | `fxc/src/fxc` |
| fxi | `fxi/src/fxi` |
| map | `map/map` |
| fpg | `fpg/fpg` |

Windows cross-compile (MinGW-w64) needs `MINGW_PREFIX`, `SDL_CONFIG`, and `--host=x86_64-w64-mingw32`. Prefer `scripts/docker-build.sh windows`.

## Docs

Generate the Doxygen site (Docker image `fenix-doxygen`, or local `doxygen`):

```shell
bash doc/stage-site.sh dist/pages
```

API HTML lands in `doc/html/` (copied to `dist/pages/docs/`). GitHub Pages deploys that tree from `main` after CI.

## CI/CD

GitHub Actions (`.github/workflows/ci.yml`):

- Linux x86_64 and Windows x86_64 — one toolchain image per OS, then static + shared (`scripts/docker-build.sh`)
- macOS — native runner, static + shared via `scripts/macos-build.sh`
- Tags — zip each `fenix-<os>[-mingw]-<static|shared>` artifact with binaries at the zip root
- Pages — `.github/workflows/pages.yml` after a successful `CI` run on `main`
