#!/bin/bash
# Golden test runner - called by ctest
# Usage: golden_test_runner.sh <alphabet_binary> <source_file> <expected_file>
ALPHABET="$1"
SOURCE="$2"
EXPECTED="$3"

OUTPUT=$("$ALPHABET" "$SOURCE" 2>/dev/null)
EXPECTED_CONTENT=$(cat "$EXPECTED")

if [ "$OUTPUT" = "$EXPECTED_CONTENT" ]; then
    exit 0
else
    echo "Expected:"
    echo "$EXPECTED_CONTENT"
    echo "Got:"
    echo "$OUTPUT"
    exit 1
fi
