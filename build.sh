#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${YELLOW}Starting build process...${NC}"

# Check if we're in the correct directory
if [[ ! -f "CMakeLists.txt" ]]; then
    echo -e "${RED}Error: CMakeLists.txt not found!${NC}"
    echo "Please run this script from the project root directory."
    exit 1
fi

# Create deps cache directory if it doesn't exist
if [ ! -d "deps_cache" ]; then
    echo -e "${YELLOW}Creating dependencies cache directory...${NC}"
    mkdir deps_cache
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir build
fi

# Enter build directory
cd build

# Configure with CMake
echo -e "${YELLOW}Configuring with CMake...${NC}"
cmake -DCMAKE_BUILD_TYPE=Debug \
      -DCMAKE_DEPS_CACHE_DIR=../deps_cache \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      .. || { echo -e "${RED}CMake configuration failed!${NC}"; exit 1; }

# Build the project using all available cores
echo -e "${YELLOW}Building project...${NC}"
cmake --build . --config Debug --parallel || { echo -e "${RED}Build failed!${NC}"; exit 1; }

# Run the tests
echo -e "${YELLOW}Running tests...${NC}"
./bin/Debug/unit_tests.exe || { echo -e "${RED}Tests failed!${NC}"; exit 1; }

# If we get here, tests passed
echo -e "${GREEN}All tests passed!${NC}"