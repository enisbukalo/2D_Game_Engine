#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Full path to clang-format (Windows style)
CLANG_FORMAT="C:\\msys64\\mingw64\\bin\\clang-format.exe"

# Function to format files
format_files() {
    local check_only=$1
    local format_command="$CLANG_FORMAT -style=file"

    if [ "$check_only" = true ]; then
        format_command="$format_command --dry-run --Werror"
        echo -e "${YELLOW}Checking formatting...${NC}"
    else
        format_command="$format_command -i"
        echo -e "${YELLOW}Formatting files...${NC}"
    fi

    # Find and format files
    find . -type f \( -name "*.cpp" -o -name "*.h" \) \
        -not -path "./build/*" \
        -not -path "./deps_cache/*" \
        -not -path "./package/*" \
        -not -path "./example_project/*" \
        -exec $format_command {} +

    local result=$?
    if [ $result -eq 0 ]; then
        if [ "$check_only" = true ]; then
            echo -e "${GREEN}All files are properly formatted!${NC}"
        else
            echo -e "${GREEN}Formatting complete!${NC}"
        fi
    else
        if [ "$check_only" = true ]; then
            echo -e "${RED}Some files need formatting!${NC}"
        else
            echo -e "${RED}Formatting failed!${NC}"
        fi
        exit 1
    fi
}

# Check for --check argument
if [ "$1" = "--check" ]; then
    format_files true
else
    format_files false
fi