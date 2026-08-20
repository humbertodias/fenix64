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

Default linkage is **static**. Images are toolchains only (Ubuntu 22.04, SDL 1.2). `scripts/docker-build.sh` builds the image, then `docker run` with the repo mounted at `/src` and autoconf/`make`. GitHub Actions uses the same Dockerfiles (`docker/build-push-action` + the same wrapper). Linux artifacts are **x86_64** (`--platform linux/amd64`).

```shell
bash scripts/docker-build.sh linux shell
bash scripts/docker-build.sh windows shell
```

macOS binaries cannot be produced from Linux containers (Apple SDK). Use a Mac or the `macos-latest` GitHub Actions job.

## macOS (native)

Needs Homebrew (`sdl12-compat`, `libpng`, `giflib`, `pkg-config`) and SDL_mixer 1.2 built from source (CI uses `--disable-music-native-midi`).

```shell
brew install sdl12-compat libpng giflib pkg-config
# then build SDL_mixer 1.2.12 into deps/local, as in .github/workflows/ci.yml
./configure --enable-fpg --enable-map
make
```

Static: `LDFLAGS=-Wl,-search_paths_first`. Shared copies `deps/local/lib/*.dylib` next to the binaries. Staged output: `dist/macos-{static,shared}/`.

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
- macOS — native runner (static + shared in one job)
- Tags — zip each `fenix-<os>[-mingw]-<static|shared>` artifact with binaries at the zip root
- Pages — `.github/workflows/pages.yml` after a successful `CI` run on `main`
