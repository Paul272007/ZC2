# ZC

## Dependencies

- Clang/LLVM
- A Linux-like OS
- Cmake

## Installing

Simply run `./install.sh` with root privileges from the root of the repository.

## Uninstalling

Simply run `./uninstall.sh` with root privileges from the root of the repository.

## Usage

You can compile and run C or C++ source files without having to worry about linking libraries with `zc run <files>`

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
