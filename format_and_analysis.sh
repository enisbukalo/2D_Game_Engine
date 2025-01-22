#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Full path to clang-format (Windows style)
CLANG_FORMAT="C:\\msys64\\mingw64\\bin\\clang-format.exe"
CPP_CHECK="C:\\msys64\\mingw64\\bin\\cppcheck.exe"

# Function to format files
format_files() {
    echo -e "${YELLOW}Formatting files...${NC}"

    # First check which files need formatting
    files_to_format=$(find . -type f \( -name "*.cpp" -o -name "*.h" \) \
        -not -path "./build/*" \
        -not -path "./deps_cache/*" \
        -not -path "./package/*" \
        -not -path "./example_project/*")

    # Format each file that needs it
    for file in $files_to_format; do
        echo "Formatting $file..."
        $CLANG_FORMAT -style=file -i "$file"
    done

    # Verify formatting
    find . -type f \( -name "*.cpp" -o -name "*.h" \) \
        -not -path "./build/*" \
        -not -path "./deps_cache/*" \
        -not -path "./package/*" \
        -not -path "./example_project/*" \
        -exec $CLANG_FORMAT -style=file --dry-run --Werror {} +

    local result=$?
    if [ $result -eq 0 ]; then
        echo -e "${GREEN}Formatting complete!${NC}"
    else
        echo -e "${RED}Formatting verification failed!${NC}"
        exit 1
    fi
}

static_analysis() {
    echo -e "${YELLOW}Running static analysis...${NC}"

    $CPP_CHECK --enable=all --error-exitcode=1 --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=noExplicitConstructor --suppress=unmatchedSuppression --suppress=missingInclude --inline-suppr --std=c++17 -I include -I include/components -I include/systems src/ include/ include/components/ include/systems/

    local result=$?
    if [ $result -eq 0 ]; then
        echo -e "${GREEN}Static analysis complete!${NC}"
    else
        echo -e "${RED}Static analysis failed!${NC}"
        exit 1
    fi
}

format_files
static_analysis