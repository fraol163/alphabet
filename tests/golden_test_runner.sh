#!/bin/bash
ALPHABET="$1"

if [ "$2" = "--dump-bytecode" ]; then
    SOURCE="$3"
    EXPECTED="$4"
    OUTPUT=$("$ALPHABET" --dump-bytecode "$SOURCE" 2>/dev/null | tr -d '\r')
else
    SOURCE="$2"
    EXPECTED="$3"
    OUTPUT=$("$ALPHABET" "$SOURCE" 2>/dev/null | tr -d '\r')
fi

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
