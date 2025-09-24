#!/bin/bash
set -e

echo "Building project..."
mkdir -p build
cd build
cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=1 ..
make -j$(nproc)

# Copy compile_commands.json for LSP support
if [ -f compile_commands.json ]; then
  cp compile_commands.json ..
fi

echo "Build completed successfully!"
