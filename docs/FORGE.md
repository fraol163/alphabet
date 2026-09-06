# Alphabet Forge: Universal Meta-Compiler & Language Creation Platform

Alphabet Forge is a comprehensive meta-compiler engine and language operating system built directly into Alphabet. It enables language designers and software engineers to create, compile, package, and distribute fully customized programming languages in minutes from a single declarative specification file (`.forge`).

---

## 1. Architectural Overview

Alphabet Forge operates on six foundational pillars:

```
                  ┌─────────────────────────────────────┐
                  │          Language Spec              │
                  │             (.forge)                │
                  └──────────────────┬──────────────────┘
                                     │
           ┌─────────────────────────┼─────────────────────────┐
           ▼                         ▼                         ▼
   ┌───────────────┐         ┌───────────────┐         ┌───────────────┐
   │ Header Engine │         │ Brand Engine  │         │ PEG Engine    │
   │ (5 styles)    │         │ (ASCII/FIGlet)│         │ (Packrat/AST) │
   └───────┬───────┘         └───────┬───────┘         └───────┬───────┘
           │                         │                         │
           └─────────────────────────┼─────────────────────────┘
                                     │
                                     ▼
           ┌───────────────────────────────────────────────────┐
           │                 Alphabet Runtime                  │
           │         (AST -> Bytecode VM & Dynamic GC)         │
           └─────────┬───────────────────────────────┬─────────┘
                     │                               │
                     ▼                               ▼
           ┌───────────────────┐           ┌───────────────────┐
           │ Stack VM Executor │           │ AOT Native C99    │
           │ (Fast Bytecode)   │           │ (Machine Binary)  │
           └───────────────────┘           └───────────────────┘
```

1. **Declarative Specification (`.forge` DSL):** Language metadata, headers, custom tokens, and PEG grammar.
2. **Multi-Style Header Engine:** Supports `#prefix`, Unix shebang (`#!/usr/bin/env`), `@pragma`, `#alphabet<lang>`, or zero-header (`none`).
3. **Packrat PEG Parser Engine:** Memoized packrat PEG parser compiling custom syntax into native Alphabet AST.
4. **Dual Execution Pipelines:**
   - **Stack Bytecode VM:** Instant, zero-compilation startup and JIT-style execution.
   - **AOT Native C99 Transpiler:** Compiles directly to standalone C99 and drives system `gcc`/`clang` to output raw machine code (ELF/Mach-O/PE).
5. **White-Labeled Standalone Exporter:** Generates branded, self-contained binary distributions without leaking Alphabet branding.
6. **Unified Developer Ecosystem:** Automated generators for VS Code extensions, test runners, code formatters, static linters, LSP servers, interactive debuggers, benchmarks, documentation sites, and in-browser web playgrounds.

---

## 2. Language Specification (`.forge` DSL)

Every custom language is defined in a `.forge` file containing five declarative blocks:

```forge
language Nova {
    version: "1.0.0"
    extension: ".nv"
    paradigm: "compiled"
    types: "dynamic"
    author: "YourName"

    header {
        style: "prefix"          // prefix | shebang | pragma | alphabet | none
        prefix: "#nova"
        required: false
        syntax: "<{version},{mode}>"
        default: "<1.0, compiled>"
    }

    banner {
        art: """
 _   _                     
| \ | | _____   ____ _     
|  \| |/ _ \ \ / / _` |    
| |\  | (_) \ V / (_| |    
|_| \_|\___/ \_/ \__,_|    
"""
        color: "cyan"            // cyan | green | yellow | red | blue | magenta | rainbow
        tagline: "The Next-Gen Compiled Language"
        show_on: ["repl", "version", "help"]
    }

    tokens {
        keyword "create"    => VAR
        keyword "display"   => PRINT
        keyword "condition" => IF
        keyword "otherwise" => ELSE
        keyword "repeat"    => WHILE
        keyword "action"    => FUNCTION
        keyword "send"      => RETURN
    }

    grammar {
        rule Program   = Statement*
        rule Statement = VarDecl | PrintStmt | IfStmt | ExprStmt
        rule VarDecl   = "create" Identifier "=" Expr ";"
        rule PrintStmt = "display" "(" Expr ")" ";"
    }
}
```

---

## 3. Creating & Forging Languages

### 3.1 Scaffolding a New Language Project
You can scaffold a full language project structure with a single command:

```bash
# Scaffold with default prefix header (#apex)
alphabet forge init Apex ./apex-lang

# Or choose header style: prefix, shebang, pragma, or none
alphabet forge init Apex ./apex-lang --style pragma
```

This creates:
- `Apex.forge`: Complete declarative specification with headers, banner, tokens, and grammar.
- `examples/hello.ape`: Runnable starter program.
- `tests/test_basic.ape`: Unit test file.
- `README.md`: Getting started guide.
- `.gitignore`: Standard ignore file.

### 3.2 Forging a Standalone Toolchain
To turn a `.forge` specification into a standalone CLI binary:

```bash
alphabet forge nova.forge --output ./bin/nova
```

This generates `./bin/nova`, a fully independent executable featuring its own custom branding, options, and commands:

```bash
./bin/nova --version
# Output: Nova Language v1.0.0 (compiled)

./bin/nova --help
# Output:
# Nova Language v1.0.0
# Usage: Nova [command] [options]
# Commands:
#   run <file>            Run a Nova program
#   debug <file>          Interactively debug a Nova program
#   compile <file> [-o b] Compile to standalone native binary (AOT)
#   repl                  Start interactive Nova REPL
#   test [dir]            Run Nova test suite
#   bench <file>          Benchmark Nova program performance
#   lint <file>           Lint Nova source file for warnings
#   lsp                   Start Nova Language Server Protocol
#   playground [dir]      Export interactive web browser playground
#   doc [dir]             Generate interactive HTML documentation
#   fmt <file>            Format Nova source file
#   pkg <init|install>    Manage packages for Nova
#   check                 Validate language specification & grammar
#   bundle [dir]          Create complete standalone distribution archive
#   --export-vscode [dir] Export VS Code syntax & extension
```

### 3.3 Exporting a Complete Distribution Bundle
To package a ready-to-distribute release archive containing the standalone compiler, VS Code extension, HTML documentation, and in-browser playground:

```bash
alphabet forge nova.forge --bundle ./dist/nova-v1.0.0
# or from the forged launcher:
./bin/nova bundle ./dist/nova-v1.0.0
```

Creates a `.tar.gz` archive and folder with:
- `bin/<lang>`: Standalone executable.
- `<Lang>.forge`: Specification.
- `vscode/`: Ready-to-install VS Code extension.
- `docs/`: Self-contained HTML documentation.
- `playground/`: Zero-server client-side web IDE (`index.html`).
- `README.md`: Distribution guide.

---

## 4. Execution & Compilation Modes

### Running with the Stack Bytecode VM
```bash
./bin/nova run program.nv
```

### Compiling to Native Machine Binaries (AOT)
```bash
# Compile to native host machine executable (ELF on Linux, Mach-O on macOS, PE on Windows)
./bin/nova compile program.nv -o ./bin/program_native

# Cross-compiling for Windows (auto-detects mingw or clang cross-compilers)
./bin/nova compile program.nv -o ./bin/program_win --target x86_64-w64-mingw32

# Cross-compiling for WebAssembly
./bin/nova compile program.nv -o ./bin/program_web --target wasm
```

### Interactive REPL with Persistent Session & Expression Auto-Printing
```bash
./bin/nova repl
```
- **Persistent State:** Variables declared across different inputs remain active throughout the session.
- **Smart Expression Printing:** Evaluates expressions directly (e.g. `x * 2` prints `200`).
- **Multi-line Input:** Automatically buffers input when opening braces `{ ... }`.
- **REPL Commands:** `.vars` (inspect state), `.clear` (reset session), `.help`, `.exit`.

### Interactive CLI & DAP Step-by-Step Debugger
```bash
./bin/nova debug program.nv
```
- **Interactive Commands:**
  - `s` / `step` / `next`: Step to next line.
  - `si`: Step one bytecode instruction.
  - `c` / `continue`: Resume execution until next breakpoint.
  - `b <line>`: Set line breakpoint.
  - `db <line>`: Remove line breakpoint.
  - `bl`: List active breakpoints.
  - `p <var>`: Inspect variable value.
  - `l` / `locals`: Display all local variables.
  - `g` / `globals`: Display all global variables.
  - `bt` / `stack`: View call stack trace.
  - `q` / `quit`: Exit debugger.
- **DAP Wire Format:** Automatically speaks Debug Adapter Protocol JSON when attached to VS Code or IDE debug clients.

---

## 5. Developer Ecosystem Generator

Alphabet Forge auto-generates the entire tooling ecosystem for any custom language:

| Tooling Command | Description | Output |
|---|---|---|
| `<lang> check` | Specification & PEG grammar integrity validator | Diagnostic report on rules, reachability, recursion |
| `<lang> --export-vscode [dir]` | Generates ready-to-publish VS Code extension | `package.json`, TextMate grammar (`.tmLanguage.json`), `language-configuration.json` |
| `<lang> test [dir]` | Built-in test runner with microsecond timings | Test execution summary with exit code |
| `<lang> fmt <file>` | AST-preserving source code formatter | Cleanly formatted source code |
| `<lang> lint <file>` | Static analysis linter | Reports unused variables, dead code, warnings |
| `<lang> lsp` | Language Server Protocol (stdio) | JSON-RPC LSP with diagnostics & autocomplete |
| `<lang> bench <file> [N]` | Execution benchmark & throughput profiler | Iteration metrics & execution statistics |
| `<lang> doc [dir]` | Responsive, dark-themed HTML documentation | `docs/index.html` with syntax guides & examples |
| `<lang> playground [dir]` | Client-side interactive web IDE | `playground/index.html` with zero-server JS execution |
| `<lang> pkg <init\|install\|list>` | Built-in package manager | Manages dependencies & manifests |

---

## 6. Reference Implementations

The Alphabet Language repository includes working reference specifications demonstrating various paradigms and header formats:

1. **`nova.forge`**: Modern compiled language with prefix header `#nova<version>`, C-style block syntax, and static AOT machine compilation.
2. **`zen.forge`**: Minimalist scripting language with Unix shebang header (`#!/usr/bin/env zen`), expressive keywords, and bytecode VM execution.
3. **`lisp.forge`**: Functional S-expression style language demonstrating `@pragma` header style (`@lisp(version=1.0)`), prefix keywords, and stack bytecode execution.
