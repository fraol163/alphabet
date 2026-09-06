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
        'forge:Universal meta-compiler to create custom languages'
    )

    _arguments -C \
        '(--help)--help[Show help]' \
        '(--version)--version[Show version]' \
        '(--debug)--debug[Enable debugger]' \
        '(--sandbox)--sandbox[Sandbox mode]' \
        '(--dump-bytecode)--dump-bytecode[Dump bytecode]' \
        '(-c --compile)'{-c,--compile}'[Compile only]' \
        '(-o --output)'{-o,--output}'[Output file]:filename:_files' \
        '(--repl)--repl[Start REPL]' \
        '(--lsp --stdio)'{--lsp,--stdio}'[Start LSP server]' \
        '1:command:(${commands})' \
        '*::arg:_files -g "*.abc"'
}

_alphabet "$@"
