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

## Commands return codes

- `0`: all good
- `1`: configuration error: `/etc/zc/conf.json` couldn't be loaded
- `2`: bad command
- `3`: no main function found
- `4`: compilation error
- `5`: file not found error
- `6`: interrupted
- `7`: error while writing file
