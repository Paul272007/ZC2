#!/bin/bash

set -e # Stop on error

# Colors
GREEN='\033[0;32m'
BLUE='\033[0;34m'
RED='\033[0;31m'
NC='\033[0m' # No Color

echo -e "${BLUE}===== ZC Installation =====${NC}"

# Detect system
echo -e "${BLUE}[0/5] Installing dependencies...${NC}"
if [ -f /etc/debian_version ]; then
  echo "Detected Debian-based system."
  sudo apt-get update -qq
  sudo apt-get install -y clang libclang-dev cmake make git libz-dev libcurl4-openssl-dev
elif [ -f /etc/redhat-release ] || [ -f /etc/fedora-release ]; then
  echo "Detected Red Hat-based system."
  sudo dnf install -y clang clang-devel cmake make git libcurl-devel zlib-devel
elif [ -f /etc/arch-release ]; then
  echo "Detected Arch-based system."
  sudo pacman -Syu --noconfirm clang cmake make git
elif [ -f /etc/os-release ] && grep -q "suse" /etc/os-release; then
  echo "Installation for openSUSE..."
  sudo zypper install -y clang clang-devel cmake make git zlib-devel libcurl-devel
else
  echo -e "${RED}Error: Unsupported operating system.${NC}"
  exit 1
fi

# Create arborescence
echo -e "${BLUE}[1/5] Creating directory structure...${NC}"
CONFIG_DIR="/etc/zc"
BIN="/usr/local/bin/zc"
DEFAULT_LIB="/usr/local/lib/zc"
DEFAULT_INCLUDE="/usr/local/include/zc"

# Clean up any existing installation and configure permissions
if [ "$EUID" -ne 0 ]; then
  sudo rm -f "$BIN"
  sudo mkdir -p "$CONFIG_DIR"
  sudo mkdir -p "$DEFAULT_LIB"
  sudo mkdir -p "$DEFAULT_INCLUDE"
  sudo chmod 666 /etc/zc/config.json
else
  rm -f "$BIN"
  mkdir -p "$CONFIG_DIR"
  mkdir -p "$DEFAULT_LIB"
  mkdir -p "$DEFAULT_INCLUDE"
  chmod 666 /etc/zc/config.json
fi

# Build and copy source files, and clean up build artifacts
if [ -d "build" ]; then
  echo -e "${BLUE}[2/5] Cleaning existing build directory...${NC}"
  rm -rf build/
fi

mkdir build/
cd build/
echo "Configuration..."
cmake -DCMAKE_BUILD_TYPE=Release ..

# Compilation
echo -e "${BLUE}[3/5] Compiling source code...${NC}"
# -j$(nproc) to use all available CPU cores
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
