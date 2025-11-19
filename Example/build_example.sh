#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Get the directory where this script is located
SCRIPT_DIR="$( cd "$( dirname "${BASH_SOURCE[0]}" )" && pwd )"

# Change to the script directory (Example_Game)
cd "$SCRIPT_DIR"

# Default values
BUILD_TYPE="Release"
BUILD_DIR="build"
CLEAN_BUILD=false
TOOLCHAIN_FILE="../cmake/toolchain-mingw64.cmake"

# Help message
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -t, --type TYPE         Set build type (Debug/Release) [default: Release]"
    echo "  -c, --clean             Clean build directory"
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
        -c|--clean)
            CLEAN_BUILD=true
            ;;
        *)
            echo -e "${RED}Unknown option: $1${NC}"
            usage
            exit 1
            ;;
    esac
    shift
done

# Normalize build type (capitalize first letter)
BUILD_TYPE="$(tr '[:lower:]' '[:upper:]' <<< ${BUILD_TYPE:0:1})$(tr '[:upper:]' '[:lower:]' <<< ${BUILD_TYPE:1})"

# Validate build type
if [[ "$BUILD_TYPE" != "Debug" && "$BUILD_TYPE" != "Release" ]]; then
    echo -e "${RED}Error: Invalid build type '$BUILD_TYPE'. Must be Debug or Release.${NC}"
    exit 1
fi

# Set GameEngine directory based on build type
GAMEENGINE_DIR="./GameEngine-Windows-$BUILD_TYPE"

# Check if CMakeLists.txt exists
if [ ! -f "CMakeLists.txt" ]; then
    echo -e "${RED}Error: CMakeLists.txt not found!${NC}"
    echo "Please run this script from the Example_Game directory."
    exit 1
fi

# Check if GameEngine library exists
if [ ! -d "$GAMEENGINE_DIR" ]; then
    echo -e "${RED}Error: GameEngine library not found at $GAMEENGINE_DIR${NC}"
    echo "Please ensure the GameEngine-Windows-$BUILD_TYPE directory exists."
    echo "You can download it using: ./update_example_game_windows.sh -t $BUILD_TYPE"
    exit 1
fi

# Check if toolchain file exists
if [ ! -f "$TOOLCHAIN_FILE" ]; then
    echo -e "${RED}Error: Toolchain file not found at $TOOLCHAIN_FILE${NC}"
    echo "Please ensure the MinGW toolchain file exists."
    exit 1
fi

# Clean build directory if requested
if [ "$CLEAN_BUILD" = true ]; then
    echo -e "${YELLOW}Cleaning build directory...${NC}"
    rm -rf ${BUILD_DIR}
fi

# Create build directory if it doesn't exist
mkdir -p ${BUILD_DIR}

# Configure project with CMake for Windows cross-compilation
echo -e "${GREEN}Configuring Example Game for Windows (MinGW)...${NC}"
cmake -B ${BUILD_DIR} \
    -G Ninja \
    -DCMAKE_TOOLCHAIN_FILE="$TOOLCHAIN_FILE" \
    -DCMAKE_BUILD_TYPE=$BUILD_TYPE \
    -DGameEngine_DIR="$GAMEENGINE_DIR/lib/cmake/GameEngine" || {
        echo -e "${RED}Configuration failed!${NC}"
        exit 1
    }

# Build project
echo -e "${GREEN}Building Example Game for Windows...${NC}"
cmake --build ${BUILD_DIR} --config $BUILD_TYPE || {
    echo -e "${RED}Build failed!${NC}"
    exit 1
}

echo -e "${GREEN}Build completed successfully!${NC}"
echo -e "${GREEN}Executable location: ${BUILD_DIR}/bin/BounceGame.exe${NC}"
echo -e "${GREEN}All required DLLs have been copied to the build directory${NC}"
