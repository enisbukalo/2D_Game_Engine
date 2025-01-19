#!/bin/bash

# Exit on error
set -e

# Remove existing build directory if it exists
rm -rf build

# Create new build directory
mkdir build

# Change to build directory
cd build

# Configure CMake in Debug mode
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build the project
cmake --build . --config Debug

echo "Build completed successfully!" 