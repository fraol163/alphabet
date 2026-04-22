#!/bin/bash
# Golden test runner - called by ctest
# Usage: golden_test_runner.sh <alphabet_binary> <source_file> <expected_file>
ALPHABET="$1"
SOURCE="$2"
EXPECTED="$3"

# Use tr to remove \r for cross-platform comparison
OUTPUT=$("$ALPHABET" "$SOURCE" 2>/dev/null | tr -d '\r')
EXPECTED_CONTENT=$(cat "$EXPECTED" | tr -d '\r')

if [ "$OUTPUT" = "$EXPECTED_CONTENT" ]; then
    exit 0
else
    echo "Expected:"
    echo "$EXPECTED_CONTENT"
    echo "Got:"
    echo "$OUTPUT"
    exit 1
fi
