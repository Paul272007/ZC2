# ZC

The last C/C++ development ecosystem you will ever need.

You can use it to compile and run single C/C++ files
or initialize, manage and build entire projects.
ZC also includes a package manager to easily install, store and manage libraries.

## Dependencies

- Clang/LLVM
- A Linux-like OS
- Cmake

## Installing

Simply run `./install.sh` with root privileges from the root of the repository.

## Uninstalling

Simply run `./uninstall.sh` with root privileges from the root of the repository.

## Usage

### Run code

`zc run <files>` compile, auto-link and run given C/C++ file(s).
`zc init <files>` initialize a new file with a content from a template.
`zc project <name>` initialize a new ZC project with the given name.
`zc build` build the current ZC project.

### Manage libraries

`zc lib create <name> <files>` create a new library with the given name and
using the given header / source / object files.
`zc lib remove <name>` uninstall the library with the given name.
`zc lib list` display all installed libraries.

Run `zc <command> --help` for more information on a specific command.

## Commands return codes

- `-1`: No idea what the error was (unexpected error)
- `0`: Success
- `10`: Compilation error
- `11`: Program execution error
- `20`: File not found
- `21`: File writing error
- `22`: File parsing error
- `30`: Configuration file parsing error
- `31`: Configuration file not found
- `32`: Configuration file reading error
- `33`: Configuration file writing writing error
- `34`: Configuration file is not filled correctly
- `40`: Bad command
- `41`: Unsupported language given
- `42`: Incompatible flags given
- `50`: Keyboard interrupt
- `51`: Operations were aborted
- `60`: Internal error
- `70`: Package not found
- `80`: No source files were found in the src/ directory
- `81`: You are currently not in a ZC project directory
- `90`: Git error
- `91`: CMake error
