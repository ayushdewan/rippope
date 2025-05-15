#!/bin/bash
set -e

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    mkdir build
fi

# Enter the build directory
cd build

# Configure with CMake
cmake ..

# Build the project
cmake --build . -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)

echo "Build completed successfully!"
echo "Run the application with: ./clay_test"
echo "Run tests with: ./piece_table_test tests/test.txt"