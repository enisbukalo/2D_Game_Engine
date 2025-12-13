#!/bin/bash
set -euo pipefail

MIN_FUNCTION_COVERAGE="${MIN_FUNCTION_COVERAGE:-80}"
LCOV_INFO_FILE="${LCOV_INFO_FILE:-coverage_filtered.info}"

usage() {
  cat <<EOF
Usage: $0 [--min <percent>] [--info <lcov_info_file>]

Checks function coverage in an lcov .info file and exits non-zero if coverage
is below the configured threshold.

Environment variables:
  MIN_FUNCTION_COVERAGE  Default minimum percentage (default: 80)
  LCOV_INFO_FILE         Default lcov .info file (default: coverage_filtered.info)
EOF
}

while [[ $# -gt 0 ]]; do
  case "$1" in
    -h|--help)
      usage
      exit 0
      ;;
    --min)
      MIN_FUNCTION_COVERAGE="$2"
      shift
      ;;
    --info)
      LCOV_INFO_FILE="$2"
      shift
      ;;
    *)
      echo "Unknown argument: $1" >&2
      usage >&2
      exit 2
      ;;
  esac
  shift
done

if ! command -v lcov >/dev/null 2>&1; then
  echo "ERROR: lcov is not installed; cannot enforce coverage threshold." >&2
  exit 1
fi

if [[ ! -f "$LCOV_INFO_FILE" ]]; then
  echo "ERROR: lcov info file not found: $LCOV_INFO_FILE" >&2
  echo "Expected this file to be produced by the Linux build/test coverage step." >&2
  exit 1
fi

summary_output="$(lcov --summary "$LCOV_INFO_FILE")"

# Example line:
#   functions..: 70.6% (233 of 330 functions)
function_line="$(printf '%s\n' "$summary_output" | grep -E '^\s*functions\.+:' | head -n 1 || true)"

if [[ -z "$function_line" ]]; then
  echo "ERROR: Could not find function coverage in lcov summary." >&2
  echo "lcov output:" >&2
  echo "$summary_output" >&2
  exit 1
fi

function_percent="$(printf '%s' "$function_line" | sed -E 's/.*functions\.+:\s*([0-9]+(\.[0-9]+)?)%.*/\1/')"

# Validate parsed percentage
if ! [[ "$function_percent" =~ ^[0-9]+(\.[0-9]+)?$ ]]; then
  echo "ERROR: Failed to parse function coverage percentage from: $function_line" >&2
  exit 1
fi

# Compare as floats using awk
awk -v got="$function_percent" -v min="$MIN_FUNCTION_COVERAGE" '
  BEGIN {
    if (got + 0 < min + 0) {
      printf("Function coverage %.1f%% is below required %.1f%%\n", got, min) > "/dev/stderr";
      exit 1;
    }
    printf("Function coverage %.1f%% meets required %.1f%%\n", got, min);
    exit 0;
  }
'
