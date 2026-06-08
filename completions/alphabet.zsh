#compdef alphabet

_alphabet() {
    local -a commands
    commands=(
        'run:Run an Alphabet program'
        'watch:Watch and re-run on changes'
        'init:Create new project'
        'test:Run tests'
        'learn:Interactive learning'
        'fmt:Format source code'
        'lint:Lint source code'
        'lsp:Start LSP server'
        'update:Self-update'
    )

    _arguments -C         '(--help)--help[Show help]'         '(--version)--version[Show version]'         '(--debug)--debug[Enable debugger]'         '(--sandbox)--sandbox[Sandbox mode]'         '(--dump-bytecode)--dump-bytecode[Dump bytecode]'         '(--profile)--profile[Profile execution]'         '(-c --compile)'{-c,--compile}'[Compile only]'         '1:command:(${commands})'         '*::arg:_files -g "*.abc"'
}

_alphabet "$@"
