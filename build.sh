#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Default values
BUILD_TYPE="Debug"
BUILD_SHARED="ON"
RUN_TESTS=true
CLEAN_BUILD=false
INSTALL_PREFIX="./package"

# Help message
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -t, --type TYPE         Set build type (Debug/Release)"
    echo "  -s, --shared            Build shared library"
    echo "  -n, --no-tests          Skip building and running tests"
    echo "  -c, --clean             Clean build directory"
    echo "  -i, --install-prefix    Set install prefix (default: ./package)"
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
            shift
            ;;
        -s|--shared)
            BUILD_SHARED="ON"
            ;;
        -n|--no-tests)
            RUN_TESTS=false
            ;;
        -c|--clean)
            CLEAN_BUILD=true
            ;;
        -i|--install-prefix)
            INSTALL_PREFIX="$2"
            shift
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            usage
            exit 1
            ;;
    esac
    shift
done

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
mkdir -p build

# Configure project with CMake
echo -e "${GREEN}Configuring project...${NC}"
cmake -B build -DCMAKE_BUILD_TYPE=$BUILD_TYPE -DGAMEENGINE_BUILD_SHARED=$BUILD_SHARED || { echo -e "${RED}Configuration failed!${NC}"; exit 1; }

# Build project
echo -e "${GREEN}Building project...${NC}"
cmake --build build --config $BUILD_TYPE || { echo -e "${RED}Build failed!${NC}"; exit 1; }

# Run tests if enabled
if [ "$RUN_TESTS" = true ]; then
    echo -e "${GREEN}Running tests...${NC}"
    # Create Testing directory structure
    mkdir -p build/Testing/Temporary

    # Run tests and capture output
    "./build/bin/${BUILD_TYPE}/unit_tests.exe" --gtest_output="xml:build/test_results.xml" 2>&1 | tee build/Testing/Temporary/LastTest.log || { echo -e "${RED}Tests failed!${NC}"; exit 1; }
fi

# Install library using CMake
echo -e "${GREEN}Installing library to ${INSTALL_PREFIX}...${NC}"
cmake --install build --prefix "$INSTALL_PREFIX" || { echo -e "${RED}Installation failed!${NC}"; exit 1; }

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}Library installed to: ${INSTALL_PREFIX}${NC}"