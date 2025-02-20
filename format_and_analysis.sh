#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Check if clang and cpp-check are installed
if ! command -v clang &> /dev/null; then
    echo -e "${RED}clang could not be found!${NC}"
    exit 1
fi

if ! command -v cppcheck &> /dev/null; then
    echo -e "${RED}cppcheck could not be found!${NC}"
    exit 1
fi


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
        clang-format -style=file -i "$file"
    done

    # Verify formatting
    find . -type f \( -name "*.cpp" -o -name "*.h" \) \
        -not -path "./build/*" \
        -not -path "./deps_cache/*" \
        -not -path "./package/*" \
        -not -path "./example_project/*" \
        -exec clang-format -style=file --dry-run --Werror {} +

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

    cppcheck --enable=all --error-exitcode=1 --check-level=exhaustive --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=noExplicitConstructor --suppress=unmatchedSuppression --suppress=missingInclude --inline-suppr --std=c++17 -I include -I include/components -I include/systems src/ include/ include/components/ include/systems/

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