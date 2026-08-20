[![CI](https://github.com/humbertodias/fenix64/actions/workflows/ci.yml/badge.svg)](https://github.com/humbertodias/fenix64/actions/workflows/ci.yml)
[![GitHub Pages](https://github.com/humbertodias/fenix64/actions/workflows/pages.yml/badge.svg)](https://github.com/humbertodias/fenix64/actions/workflows/pages.yml)
[![Ask DeepWiki](https://deepwiki.com/badge.svg)](https://deepwiki.com/humbertodias/fenix64)
![GitHub all releases](https://img.shields.io/github/downloads/humbertodias/fenix64/total)

# Fenix Project 1.0

Interpreted script language for 2D games: compiler `fxc`, interpreter `fxi`, and the `map` / `fpg` graphic tools.

- [API reference](https://humbertodias.github.io/fenix64/docs/) — Doxygen docs for `fxc`, `fxi`, `map`, and `fpg` (published from `main` via GitHub Pages).

## Install

The installer defaults to a **static** build.

Linux / macOS / Git Bash:

```shell
curl -sL "https://raw.githubusercontent.com/humbertodias/fenix64/main/scripts/install.sh" | bash
```

Shared libraries:

```shell
curl -sL "https://raw.githubusercontent.com/humbertodias/fenix64/main/scripts/install.sh" | FENIX_LINKAGE=shared bash
```

Windows (PowerShell):

```powershell
irm https://raw.githubusercontent.com/humbertodias/fenix64/main/scripts/install.ps1 | iex
```

Shared DLLs:

```powershell
$env:FENIX_LINKAGE = "shared"
irm https://raw.githubusercontent.com/humbertodias/fenix64/main/scripts/install.ps1 | iex
```

It installs `fxc`, `fxi`, `map`, and `fpg` into `$HOME/fenix` (`FENIX_HOME`) and prepends that directory to `PATH`.

| Variable | Default | Description |
|----------|---------|-------------|
| `FENIX_HOME` | `$HOME/fenix` | Install directory |
| `FENIX_VERSION` | `latest` | Release tag |
| `FENIX_LINKAGE` | `static` | `static` or `shared` archive |
| `FENIX_REPO` | `humbertodias/fenix64` | GitHub repository |

## Project Structure

```
/<FENIXROOT>
  |
  |- FENIX (contenido de este módulo de CVS)
  |- SDL (ficheros necesarios de la SDL DEVEL)
  |  |- INCLUDE
  |  |- LIB
  |- SDL_MIXER (ficheros necesarios de la SDL_MIXER)
  |  |- INCLUDE
  |  |- LIB
  |- ZLIB (ficheros necesarios de la ZLIB)
  |  |- INCLUDE
  |  |- LIB
  |- LIBPNG (ficheros necesarios de libpng)
  |- LIBUNGIF (si quieres compilar MAP.EXE necesitas la LIBUNGIF)
```

See [BUILDING.md](BUILDING.md) for Docker outputs, macOS (Homebrew), and native autoconf/`make`.
