#!/bin/bash

if [ ! -f "build/install_manifest.txt" ]; then
  echo "Error: install_manifest.txt not found. Please run the install script first."
  exit 1
fi

echo "Deleting installed files..."
xargs sudo rm -f < build/install_manifest.txt

echo "Uninstallation complete."
