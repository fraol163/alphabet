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

    # Strip line comments before counting braces so braces inside
    # `// ... } ...` don't break the depth tracking. Use '#' as a
    # sentinel for stripped content.
    while IFS= read -r line || [ -n "$line" ]; do
        line_num=$((line_num + 1))
        trimmed="${line#"${line%%[![:space:]]*}"}"  # bash-native trim

        if [ $line_num -eq 1 ]; then
            if [[ "$trimmed" =~ ^#alphabet\< ]]; then
                has_header=true
                lang="${trimmed#\#alphabet<}"
                lang="${lang%%>*}"
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

        # Strip comments (`// ...` to end-of-line) before brace counting.
        # The lexer recognizes `//` and `///` (docstring) as line comments.
        brace_line="${line%%//*}"
        for (( i=0; i<${#brace_line}; i++ )); do
            c="${brace_line:$i:1}"
            case "$c" in
                '{') brace_depth=$((brace_depth + 1)) ;;
                '}') brace_depth=$((brace_depth - 1)) ;;
            esac
        done

        # Detect non-English import keywords. The single-letter `x`
        # is the canonical form, but each language also accepts its
        # full keyword (import/importar/importer/importieren/አስገባ).
        # Only flag `*` which is genuinely not an import syntax.
        if [[ "$trimmed" =~ ^\* ]]; then
            echo "ERROR: $file:$line_num: Use 'x' (or import/importar/etc.) not '*'"
            ERRORS=$((ERRORS + 1))
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
