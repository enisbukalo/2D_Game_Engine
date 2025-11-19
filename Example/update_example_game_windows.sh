#!/bin/bash

# Script to update Example_Game with the latest Windows build from GitHub Actions
# This downloads the latest GameEngine-Windows artifact and replaces the local copy

set -e  # Exit on error

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
EXAMPLE_GAME_DIR="$SCRIPT_DIR"
BUILD_TYPE="Release"  # Default to Release
REPO="enisbukalo/2D_Game_Engine"

# Help message
usage() {
    echo "Usage: $0 [options]"
    echo "Options:"
    echo "  -h, --help              Show this help message"
    echo "  -t, --type TYPE         Set build type (Debug/Release) [default: Release]"
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
        *)
            echo "Unknown option: $1"
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
    echo "Error: Invalid build type '$BUILD_TYPE'. Must be Debug or Release."
    exit 1
fi

ARTIFACT_NAME="GameEngine-Windows-$BUILD_TYPE"
TARGET_DIR="$EXAMPLE_GAME_DIR/GameEngine-Windows-$BUILD_TYPE"

echo "========================================="
echo "Updating Example_Game Windows Build"
echo "========================================="
echo ""

# Check if gh CLI is installed
if ! command -v gh &> /dev/null; then
    echo "Error: GitHub CLI (gh) is not installed"
    echo "Install it from: https://cli.github.com/"
    exit 1
fi

# Check if authenticated
if ! gh auth status &> /dev/null; then
    echo "Error: Not authenticated with GitHub CLI"
    echo "Run: gh auth login"
    exit 1
fi

# Get the latest successful workflow run for CI (which calls build-windows)
echo "Finding latest successful CI workflow run..."
RUN_ID=$(gh run list \
    --repo="$REPO" \
    --workflow=ci.yml \
    --status=success \
    --limit=1 \
    --json databaseId \
    --jq '.[0].databaseId')

if [ -z "$RUN_ID" ]; then
    echo "Error: No successful CI workflow runs found"
    echo "Tip: Make sure you have a PR with passing CI checks"
    exit 1
fi

echo "Latest successful run ID: $RUN_ID"
echo ""

# Download the artifact
echo "Downloading artifact: $ARTIFACT_NAME"
TEMP_DIR=$(mktemp -d)
cd "$TEMP_DIR"

gh run download "$RUN_ID" --repo="$REPO" --name "$ARTIFACT_NAME"

echo "Artifact downloaded to: $TEMP_DIR"
echo ""

# Remove old GameEngine-Windows directory
if [ -d "$TARGET_DIR" ]; then
    echo "Removing old GameEngine-Windows-$BUILD_TYPE..."
    rm -rf "$TARGET_DIR"
fi

# Create target directory and move contents
echo "Installing new GameEngine-Windows-$BUILD_TYPE..."
mkdir -p "$TARGET_DIR"
mv bin include lib "$TARGET_DIR/"

# Cleanup
cd "$EXAMPLE_GAME_DIR"
rm -rf "$TEMP_DIR"

echo ""
echo "========================================="
echo "Update Complete!"
echo "========================================="
echo "Location: $TARGET_DIR"
echo ""
echo "Next steps:"
echo "  ./build_example.sh"
