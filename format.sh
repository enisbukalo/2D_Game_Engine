#!/bin/bash

# Colors for output
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

# Full path to clang-format (Windows style)
CLANG_FORMAT="C:\\msys64\\mingw64\\bin\\clang-format.exe"

echo -e "${YELLOW}Formatting C++ files...${NC}"

# Find all .cpp and .h files and format them
find . -type f \( -name "*.cpp" -o -name "*.h" \) -not -path "./build/*" -not -path "./deps_cache/*" | while read -r file; do
    echo -e "Formatting ${file}..."
    "$CLANG_FORMAT" -i -style=file "$file"
done

echo -e "${GREEN}Formatting complete!${NC}" 