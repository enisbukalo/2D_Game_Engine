#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default configuration
BUILD_TYPE="Debug"
BUILD_SHARED=OFF
BUILD_TESTS=ON
CLEAN_BUILD=false
INSTALL_PREFIX=""

# Usage function
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -h, --help           Show this help message"
    echo "  -t, --type TYPE      Build type (Debug|Release) [default: Debug]"
    echo "  -s, --shared         Build as shared library [default: OFF]"
    echo "  --no-tests           Disable building tests"
    echo "  -c, --clean          Clean build directory before building"
    echo "  -i, --install PREFIX Install to specified prefix"
}

# Parse command line arguments
while [[ $# -gt 0 ]]; do
    case $1 in
        -h|--help)
            usage
            exit 0
            ;;
        -t|--type)
            BUILD_TYPE="$2"
            shift 2
            ;;
        -s|--shared)
            BUILD_SHARED=ON
            shift
            ;;
        --no-tests)
            BUILD_TESTS=OFF
            shift
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            shift
            ;;
        -i|--install)
            INSTALL_PREFIX="$2"
            shift 2
            ;;
        *)
            echo "Unknown option: $1"
            usage
            exit 1
            ;;
    esac
done

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo -e "${RED}Invalid build type: $BUILD_TYPE${NC}"
    echo "Build type must be either Debug or Release"
    exit 1
fi

# Check if CMakeLists.txt exists
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found!${NC}"
    echo "Please run this script from the project root directory."
    exit 1
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf build
fi

# Create build directory if it doesn't exist
if [ ! -d "build" ]; then
    echo -e "${YELLOW}Creating build directory...${NC}"
    mkdir build
fi

# Navigate to build directory
cd build || exit 1

echo -e "${YELLOW}Configuring with CMake...${NC}"
# Configure with CMake
cmake -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
      -DGAMEENGINE_BUILD_SHARED=$BUILD_SHARED \
      -DGAMEENGINE_BUILD_TESTS=$BUILD_TESTS \
      -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
      ${INSTALL_PREFIX:+-DCMAKE_INSTALL_PREFIX="$INSTALL_PREFIX"} \
      .. || { echo -e "${RED}CMake configuration failed!${NC}"; exit 1; }

echo -e "${YELLOW}Building project...${NC}"
# Build dependencies first
echo -e "${YELLOW}Building dependencies...${NC}"
cmake --build . --config $BUILD_TYPE --target sfml-system sfml-window sfml-graphics ImGui-SFML || { echo -e "${RED}Dependencies build failed!${NC}"; exit 1; }

# Build the main project
echo -e "${YELLOW}Building main project...${NC}"
cmake --build . --config $BUILD_TYPE --target GameEngine || { echo -e "${RED}Build failed!${NC}"; exit 1; }

# Build and run tests if enabled
if [ "$BUILD_TESTS" = "ON" ]; then
    echo -e "${YELLOW}Building tests...${NC}"
    cmake --build . --config $BUILD_TYPE --target unit_tests || { echo -e "${RED}Tests build failed!${NC}"; exit 1; }

    echo -e "${YELLOW}Running tests...${NC}"
    # Create Testing directory structure
    mkdir -p Testing/Temporary

    # Run tests and capture output to both console and file
    "./bin/${BUILD_TYPE}/unit_tests.exe" --gtest_output="xml:test_results.xml" 2>&1 | tee Testing/Temporary/LastTest.log || { echo -e "${RED}Tests failed!${NC}"; exit 1; }
fi

# Install if prefix is specified
if [ -n "$INSTALL_PREFIX" ]; then
    echo -e "${YELLOW}Installing to $INSTALL_PREFIX...${NC}"
    cmake --install . --config $BUILD_TYPE || { echo -e "${RED}Installation failed!${NC}"; exit 1; }
fi

echo -e "${GREEN}Build completed successfully!${NC}"