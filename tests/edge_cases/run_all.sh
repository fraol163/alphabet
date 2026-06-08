#!/bin/bash
# Edge case test runner for Alphabet Language
BINARY="/home/fraol/Desktop/All In One/Alphabet_Language/build/alphabet"
TEST_DIR="/home/fraol/Desktop/All In One/Alphabet_Language/tests/edge_cases"

PASS_COUNT=0
FAIL_COUNT=0

echo "========================================"
echo " Alphabet Language Edge Case Test Suite"
echo "========================================"
echo ""

while IFS= read -r f; do
    fname=$(basename "$f" .abc)
    
    # Run with timeout
    OUTPUT=$(timeout 10 "$BINARY" "$f" 2>&1)
    EXIT_CODE=$?
    
    if [ "$EXIT_CODE" -eq 124 ]; then
        echo "FAIL: $fname (TIMEOUT)"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    elif [ "$EXIT_CODE" -ne 0 ]; then
        echo "FAIL: $fname (EXIT=$EXIT_CODE)"
        echo "  $OUTPUT"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    elif echo "$OUTPUT" | grep -q "PASS:"; then
        if echo "$OUTPUT" | grep -q "FAIL:"; then
            echo "FAIL: $fname"
            echo "  $OUTPUT"
            FAIL_COUNT=$((FAIL_COUNT + 1))
        else
            PASS_LINE=$(echo "$OUTPUT" | grep "PASS:" | head -1)
            echo "PASS: $fname -> $PASS_LINE"
            PASS_COUNT=$((PASS_COUNT + 1))
        fi
    elif echo "$OUTPUT" | grep -q "FAIL:"; then
        echo "FAIL: $fname"
        echo "  $OUTPUT"
        FAIL_COUNT=$((FAIL_COUNT + 1))
    else
        echo "PASS: $fname (ran OK, no assertion)"
        PASS_COUNT=$((PASS_COUNT + 1))
    fi
done < <(find "$TEST_DIR" -name "*.abc" -type f | sort)

echo ""
echo "========================================"
echo " Results: $PASS_COUNT PASS / $FAIL_COUNT FAIL"
echo " Total: $((PASS_COUNT + FAIL_COUNT)) tests"
echo "========================================"
