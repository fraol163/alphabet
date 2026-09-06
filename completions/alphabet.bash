#!/bin/bash
_alphabet_completions() {
    local cur prev commands
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    commands="run test init info doc bench examples tour lint voice-tutorial pkg update forge"

    if [[ ${cur} == -* ]]; then
        # Flags actually supported by the alphabet binary (verified 2026-09-04).
        COMPREPLY=( $(compgen -W "--help --version --debug --sandbox --dump-bytecode -c --compile -o --output --repl --lsp --stdio" -- ${cur}) )
        return 0
    fi

    if [[ ${prev} == "alphabet" ]]; then
        COMPREPLY=( $(compgen -W "${commands}" -- ${cur}) )
        return 0
    fi

    # File completion for subcommands that take files
    case "${prev}" in
        run|watch|fmt|lint|lsp)
            COMPREPLY=( $(compgen -f -X '!*.abc' -- ${cur}) )
            return 0
            ;;
        forge)
            COMPREPLY=( $(compgen -W "init --check --bundle --export-vscode --test --fmt --pkg --repl" -f -X '!*.forge' -- ${cur}) )
            return 0
            ;;
    esac
}
complete -F _alphabet_completions alphabet
