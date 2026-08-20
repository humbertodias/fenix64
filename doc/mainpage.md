# Fenix

Fenix is an interpreted script language for 2D games: a compiler (`fxc`), an
interpreter (`fxi`), and the `map` / `fpg` graphic tools.

This reference is generated automatically from the C sources with
[Doxygen](https://www.doxygen.nl/).

## Components

| Binary | Role |
|--------|------|
| `fxc` | Compiler — turns `.prg` sources into `.dcb` bytecode |
| `fxi` | Interpreter — loads `.dcb` and runs the game |
| `map` | MAP / FBM graphic tool |
| `fpg` | FPG graphic library tool |

## Layout

- **include/** — shared headers (`dcb.h`, `files.h`, graphic formats)
- **common/** — file I/O, ctypes, directories
- **fxc/** — compiler
- **fxi/** — interpreter and 2D engine
- **map/**, **fpg/** — graphic utilities

## User documentation

Project pages (README, LEEME, file formats) are published with this API
reference on GitHub Pages.
