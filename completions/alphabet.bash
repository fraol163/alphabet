#!/bin/bash
_alphabet_completions() {
    local cur prev commands
    COMPREPLY=()
    cur="${COMP_WORDS[COMP_CWORD]}"
    prev="${COMP_WORDS[COMP_CWORD-1]}"
    commands="run watch init test learn fmt lint lsp update"

    if [[ ${cur} == -* ]]; then
        COMPREPLY=( $(compgen -W "--help --version --debug --sandbox --dump-bytecode --profile -c --compile" -- ${cur}) )
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
    esac
}
complete -F _alphabet_completions alphabet
