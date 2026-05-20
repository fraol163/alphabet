#!/bin/bash
set -euo pipefail

if [ $# -eq 0 ]; then
    echo "Usage: $0 <file.abc> [file2.abc ...]"
    echo "  Lints .abc files for common issues"
    exit 1
fi

ERRORS=0
WARNINGS=0

lint_file() {
    local file="$1"
    local line_num=0
    local brace_depth=0
    local has_header=false

    if [ ! -f "$file" ]; then
        echo "ERROR: $file: File not found"
        ERRORS=$((ERRORS + 1))
        return
    fi

    while IFS= read -r line || [ -n "$line" ]; do
        line_num=$((line_num + 1))
        trimmed="$(echo "$line" | sed 's/^[[:space:]]*//')"

        if [ $line_num -eq 1 ]; then
            if echo "$trimmed" | grep -qE '^#alphabet<'; then
                has_header=true
                lang="$(echo "$trimmed" | sed 's/#alphabet<\([^>]*\)>.*/\1/')"
                case "$lang" in
                    en|am|es|fr|de) ;;
                    *)
                        echo "WARNING: $file:$line_num: Unknown language '$lang'"
                        WARNINGS=$((WARNINGS + 1))
                        ;;
                esac
            else
                echo "ERROR: $file:$line_num: Missing #alphabet<lang> header"
                ERRORS=$((ERRORS + 1))
            fi
        fi

        for (( i=0; i<${#line}; i++ )); do
            c="${line:$i:1}"
            case "$c" in
                '{') brace_depth=$((brace_depth + 1)) ;;
                '}') brace_depth=$((brace_depth - 1)) ;;
            esac
        done

        if echo "$trimmed" | grep -qE '^\*'; then
            echo "ERROR: $file:$line_num: Use 'x' for import, not '*'"
            ERRORS=$((ERRORS + 1))
        fi

        if echo "$trimmed" | grep -qE '^\s*//'; then
            echo "WARNING: $file:$line_num: Comments not supported in .abc files"
            WARNINGS=$((WARNINGS + 1))
        fi

    done < "$file"

    if [ "$has_header" = false ]; then
        echo "ERROR: $file: Missing #alphabet<lang> header"
        ERRORS=$((ERRORS + 1))
    fi

    if [ $brace_depth -ne 0 ]; then
        echo "ERROR: $file: Unmatched braces (depth=$brace_depth)"
        ERRORS=$((ERRORS + 1))
    fi
}

for file in "$@"; do
    lint_file "$file"
done

echo ""
echo "Lint complete: $ERRORS errors, $WARNINGS warnings"

if [ $ERRORS -gt 0 ]; then
    exit 1
fi
exit 0
