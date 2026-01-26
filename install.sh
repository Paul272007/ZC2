#!/bin/bash

set -e # Stop on error

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}========== ZC Installation ==========${NC}"

# Detect system
echo -e "${BLUE}[0/5] Installing dependencies...${NC}"
if [ -f /etc/debian_version ]; then
  echo "Detected Debian-based system."
  sudo apt-get update -qq
  sudo apt-get install -y clang libclang-dev llvm-dev cmake make git libz-dev libcurl4-openssl-dev
elif [ -f /etc/redhat-release ] || [ -f /etc/fedora-release ]; then
  echo "Detected Red Hat-based system."
  sudo dnf install -y clang clang-devel cmake make git libcurl-devel zlib-devel
elif [ -f /etc/arch-release ]; then
  echo "Detected Arch-based system."
  sudo pacman -S --needed --noconfirm clang cmake make git llvm libedit
elif [ -f /etc/os-release ] && grep -q "suse" /etc/os-release; then
  echo "Installation for openSUSE..."
  sudo zypper install -y clang clang-devel cmake make git zlib-devel libcurl-devel
else
  echo -e "${RED}Error: Unsupported operating system.${NC}"
  exit 1
fi

# Create arborescence
echo -e "${BLUE}[1/5] Setting up user environment...${NC}"
ZC_DIR="$HOME/.zc"

mkdir -p "$ZC_DIR/lib"
mkdir -p "$ZC_DIR/include"

cp etc/config.json "$ZC_DIR"
cp etc/registry.json "$ZC_DIR"

cp -r etc/templates "$ZC_DIR"
cp -r etc/project_templates "$ZC_DIR"

# Build and copy source files, and clean up build artifacts
echo -e "${BLUE}[2/5] Cleaning existing installation...${NC}"

# Clean up any existing installation
if [ "$EUID" -ne 0 ]; then
  sudo rm -f "$BIN"
else
  rm -f "$BIN"
fi

if [ -d "build" ]; then
  rm -rf build/
fi

mkdir build/
cd build/
echo "Configuration..."
cmake -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local ..

# Parallel compilation to compile faster
echo -e "${BLUE}[3/5] Compiling source code...${NC}"
cmake --build . --config Release --parallel "$(nproc)"

# Installation
echo -e "${BLUE}[4/5] Installing ZC...${NC}"
if [ "$EUID" -ne 0 ]; then
  sudo cmake --install .
else
  cmake --install .
fi

echo -e "${GREEN}===== ZC installed successfully! =====${NC}"
exit 0
