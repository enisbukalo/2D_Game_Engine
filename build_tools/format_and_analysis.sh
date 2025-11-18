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

    # Use git ls-files to respect .gitignore and only get tracked files
    files_to_format=$(git ls-files '*.cpp' '*.h')
    
    # Lets ignore all of the files in the tests/ directory
    files_to_format=$(echo "$files_to_format" | grep -v '^tests/')

    if [ -z "$files_to_format" ]; then
        echo -e "${YELLOW}No files to format${NC}"
        return
    fi

    # Format each file that needs it
    for file in $files_to_format; do
        echo "Formatting $file..."
        clang-format -style=file -i "$file"
    done

    # Verify formatting (using git ls-files again)
    git ls-files '*.cpp' '*.h' | xargs clang-format -style=file --dry-run --Werror

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

    # Create report header
    cat > static_analysis_report.md << EOF
# Static Analysis Report

**Date:** $(date)
**Tool:** cppcheck
**Configuration:** --enable=all --check-level=exhaustive

---

## Results

EOF

    # Get list of tracked source and header files
    tracked_files=$(git ls-files '*.cpp' '*.h')

    # Ignore all of the files in the tests/ directory
    tracked_files=$(echo "$tracked_files" | grep -v '^tests/')

    if [ -z "$tracked_files" ]; then
        echo -e "${YELLOW}No files to analyze${NC}"
        echo -e "\n---\n\n**Status:** ⚠️ NO FILES\n" >> static_analysis_report.md
        return
    fi

    # Run cppcheck on tracked files only
    echo "$tracked_files" | xargs cppcheck --enable=all --error-exitcode=1 --check-level=exhaustive --language=c++ --std=c++17 --suppress=missingIncludeSystem --suppress=unusedFunction --suppress=noExplicitConstructor --suppress=unmatchedSuppression --suppress=missingInclude --inline-suppr -I include -I include/components -I include/systems 2>&1 | tee -a static_analysis_report.md

    local result=${PIPESTATUS[0]}

    # Add summary to report
    if [ $result -eq 0 ]; then
        echo -e "\n---\n\n**Status:** ✅ PASSED\n" >> static_analysis_report.md
        echo -e "${GREEN}Static analysis complete! Report saved to static_analysis_report.md${NC}"
    else
        echo -e "\n---\n\n**Status:** ❌ FAILED\n" >> static_analysis_report.md
        echo -e "${RED}Static analysis failed! Report saved to static_analysis_report.md${NC}"
        exit 1
    fi
}

format_files
static_analysis