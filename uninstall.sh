#!/bin/bash

echo "Warning: this will delete all the libraries installed via ZC. Back up important data before proceeding."

if [ ! -f "build/install_manifest.txt" ]; then
  echo "Error: install_manifest.txt not found. Please run the install script first."
  exit 1
fi

echo "Deleting installed files..."
xargs sudo rm -f <build/install_manifest.txt

DEFAULT_LIB="/usr/local/lib/zc"
DEFAULT_INCLUDE="/usr/local/include/zc"

if [ "$EUID" -ne 0 ]; then
  sudo rm -rf "$DEFAULT_LIB"
  sudo rm -rf "$DEFAULT_INCLUDE"
else
  rm -rf "$DEFAULT_LIB"
  rm -rf "$DEFAULT_INCLUDE"
fi

echo "Uninstallation complete."
