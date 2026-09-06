```text

            d8b            d8b                 d8b
           88P            ?88                 ?88                d8P
          d88              88b                 88b            d888888P
 d888b8b  888  ?88,.d88b,  888888b  d888b8b    888888b  d8888b  ?88'
d8P' ?88  ?88  `?88'  ?88  88P `?8bd8P' ?88    88P `?8bd8b_,dP  88P
88b  ,88b  88b   88b  d8P d88   88P88b  ,88b  d88,  d8888b      88b
`?88P'`88b  88b  888888P'd88'   88b`?88P'`88bd88'`?88P'`?888P'  `?8b
                 88P'
                d88
                ?8P
```
# Alphabet Programming Language v2.3.6

**A High-Performance Multilingual Language and Universal Meta-Compiler Platform.**

[![Build Status](https://github.com/fraol163/alphabet/actions/workflows/ci.yml/badge.svg)](https://github.com/fraol163/alphabet/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-blue.svg)](LICENSE.txt)
[![C++ Standard](https://img.shields.io/badge/C%2B%2B-17-blue.svg)](CMakeLists.txt)
[![Platforms](https://img.shields.io/badge/platform-Linux%20%7C%20macOS%20%7C%20Windows-lightgrey.svg)](#installation)
[![Tests](https://img.shields.io/badge/tests-35%20passed%20%28100%25%29-brightgreen.svg)](#testing-and-verification)

---

## Table of Contents

- [Overview](#overview)
- [Design Philosophy and Core Concepts](#design-philosophy-and-core-concepts)
- [Language Specification and Syntax](#language-specification-and-syntax)
  - [The 19 Core Keywords](#the-19-core-keywords)
  - [Multilingual Keyword Matrix](#multilingual-keyword-matrix)
  - [Type System: Numeric Type IDs and Named Types](#type-system-numeric-type-ids-and-named-types)
  - [Builtin System Functions (z Namespace)](#builtin-system-functions-z-namespace)
- [Language Tour and Code Examples](#language-tour-and-code-examples)
  - [Variables and Type Inference](#variables-and-type-inference)
  - [Control Flow: If, Loops, and For-Each](#control-flow-if-loops-and-for-each)
  - [Pattern Matching](#pattern-matching)
  - [Functions, Closures, and Higher-Order Logic](#functions-closures-and-higher-order-logic)
  - [Object-Oriented Programming and Inheritance](#object-oriented-programming-and-inheritance)
  - [Exception Handling](#exception-handling)
  - [Concurrency and Multithreading](#concurrency-and-multithreading)
- [Standard Library](#standard-library)
- [Alphabet Forge: Universal Meta-Compiler Platform](#alphabet-forge-universal-meta-compiler-platform)
  - [Architectural Overview](#architectural-overview)
  - [The .forge Specification DSL](#the-forge-specification-dsl)
  - [Multi-Style Header Engine](#multi-style-header-engine)
  - [Dual Execution Pipelines: VM and Native AOT](#dual-execution-pipelines-vm-and-native-aot)
  - [White-Labeled Standalone Toolchains](#white-labeled-standalone-toolchains)
  - [Scaffolding Wizard (forge init)](#scaffolding-wizard-forge-init)
  - [Grammar and Spec Validator (forge --check)](#grammar-and-spec-validator-forge---check)
  - [Self-Contained Distribution Bundler (forge --bundle)](#self-contained-distribution-bundler-forge---bundle)
  - [Reference Implementations: Nova, Zen, and Lisp](#reference-implementations-nova-zen-and-lisp)
- [Installation and Building](#installation-and-building)
  - [Quick Install (Linux and macOS)](#quick-install-linux-and-macos)
  - [Windows Installation](#windows-installation)
  - [Building from Source with CMake](#building-from-source-with-cmake)
  - [Editor Support and VS Code Extension](#editor-support-and-vs-code-extension)
  - [Verifying Installation](#verifying-installation)
- [Command Line Interface Reference](#command-line-interface-reference)
  - [Alphabet CLI Commands](#alphabet-cli-commands)
  - [Alphabet Forge Subcommands](#alphabet-forge-subcommands)
  - [Interactive REPL Reference](#interactive-repl-reference)
  - [Interactive Step Debugger Reference](#interactive-step-debugger-reference)
- [Testing and Verification](#testing-and-verification)
- [What is New in v2.3.6](#what-is-new-in-v236)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

---

## Overview

**Alphabet** is a computing environment engineered in C++17 that delivers two key systems in a single unified binary:

1. **A Concept-First Multilingual Programming Language:** Designed to make computer science accessible to non-English speakers and learners worldwide. Alphabet allows programmers to think and code in English, Amharic, Spanish, French, or German. It bridges concise 1-letter keywords for high-density algorithmic reasoning with industry-standard named types, object-oriented inheritance, closures, and multithreading.
2. **Alphabet Forge (Universal Meta-Compiler Platform):** A language creation operating system that allows developers to define, parse, compile, and package entirely new domain-specific and general-purpose programming languages from a single declarative `.forge` specification. Forge generates white-labeled standalone binaries, complete VS Code extensions, LSP servers, interactive debuggers, documentation websites, and zero-server in-browser Web IDEs.

---

## Design Philosophy and Core Concepts

Traditional programming languages require students and software engineers to learn English syntax and abstract computational logic at the same time. Alphabet removes that barrier.

### Concept-First Learning

Alphabet isolates core programming concepts so that knowledge transfers directly to other languages:

| Abstract Concept | Alphabet Syntax | Transferred Knowledge |
|---|---|---|
| Labeled Storage | `int x = 10` or `5 x = 10` | C, C++, Java, Go, Rust |
| Branching Decision | `i (x > 0) { ... }` or `if (x > 0) { ... }` | Python, JavaScript, Swift |
| Repetition / Loop | `l (x < 5) { ... }` or `while (x < 5) { ... }` | Every procedural language |
| Subroutine Recipe | `m void run() { ... }` or `function void run()` | Python, JavaScript, Java |
| Blueprint Structure | `c Car { ... }` or `class Car { ... }` | C++, Java, C#, Python |
| Exception Handler | `t { ... } h (5 e) { ... }` | Java, Python, C++ |

---

## Language Specification and Syntax

### The 19 Core Keywords

Alphabet code can be written using single-letter keywords for rapid notation, or standard named keywords:

| Short | Named Keyword | Example | Description |
|---|---|---|---|
| `a` | `abstract` | `a c Shape { }` | Defines an abstract class |
| `b` | `break` | `b` | Breaks out of a loop |
| `c` | `class` | `c Account { }` | Defines a class |
| `e` | `else` | `i (x) { } e { }` | Alternative branch in conditional |
| `h` | `handle` | `h (5 err) { }` | Exception catch block |
| `i` | `if` | `i (val > 0) { }` | Conditional branch |
| `j` | `interface` | `j Printable { }` | Defines an interface |
| `k` | `continue` | `k` | Skips to the next loop iteration |
| `l` | `loop` / `while`| `l (count > 0) { }` | While or for-style loop |
| `m` | `method` / `fn` | `v m 5 sum(5 a, 5 b)` | Function or method declaration |
| `n` | `new` | `n Account()` | Instantiates an object |
| `p` | `private` | `p 5 balance = 0` | Restricts access to class scope |
| `q` | `match` | `q (x) { 1: ... }` | Pattern matching switch statement |
| `r` | `return` | `r x + y` | Returns value from function |
| `s` | `static` | `s 5 total = 0` | Defines static class member |
| `t` | `try` | `t { risky() }` | Encloses exception-prone code |
| `v` | `public` | `v m void print()` | Public access modifier |
| `x` | `import` | `x "math"` | Imports standard or local module |
| `z` | `system` | `z.o("Output")` | Standard system built-in namespace |

### Multilingual Keyword Matrix

Every Alphabet file begins with a language header declaring the dialect. Keywords automatically translate at the lexical level:

| Feature | English (`#alphabet<en>`) | Amharic (`#alphabet<am>`) | Spanish (`#alphabet<es>`) | French (`#alphabet<fr>`) | German (`#alphabet<de>`) |
|---|---|---|---|---|---|
| Class | `class` / `c` | `ክፍል` / `ክ` | `clase` / `c` | `classe` / `c` | `klasse` / `k` |
| Method | `method` / `m` | `ዘዴ` / `መ` | `metodo` / `m` | `methode` / `m` | `methode` / `m` |
| If | `if` / `i` | `ከሆነ` / `ከ` | `si` / `s` | `si` / `s` | `wenn` / `w` |
| Else | `else` / `e` | `ካልሆነ` / `ካ` | `sino` / `e` | `sinon` / `e` | `sonst` / `s` |
| Loop | `loop` / `l` | `ዙር` / `ዙ` | `bucle` / `b` | `boucle` / `b` | `schleife` / `s` |
| Return | `return` / `r` | `መልስ` / `ል` | `retorna` / `r` | `retour` / `r` | `ruckgabe` / `r` |
| New | `new` / `n` | `አዲስ` / `አ` | `nuevo` / `n` | `nouveau` / `n` | `neu` / `n` |
| Print | `z.o(...)` | `ዝ.ው(...)` | `z.o(...)` | `z.o(...)` | `z.o(...)` |

### Type System: Numeric Type IDs and Named Types

Alphabet provides compile-time static type checking with runtime type tag enforcement. Developers can use human-readable named types or compact numeric IDs:

| Type ID | Named Type | Description | Example Literal |
|---|---|---|---|
| `0` | `void` | Absence of value or return | `m void run()` |
| `1` | `i8` | 8-bit signed integer | `1 x = 127` |
| `2` | `i16` | 16-bit signed integer | `2 x = 32767` |
| `3` | `i32` | 32-bit signed integer | `3 x = 2147483647` |
| `4` | `i64` | 64-bit signed integer | `4 x = 9223372036854775807` |
| `5` | `int` | Standard integer (promotes to 64-bit) | `5 x = 42` or `int x = 42` |
| `6` | `f32` | 32-bit single-precision float | `6 x = 3.14` |
| `7` | `f64` | 64-bit double-precision float | `7 x = 2.718281828` |
| `8` | `float` | Standard float | `8 x = 1.618` or `float x = 1.618` |
| `11` | `bool` | Boolean value | `11 ok = true` or `bool ok = true` |
| `12` | `str` | UTF-8 string | `12 s = "Alphabet"` or `str s = "Text"` |
| `13` | `list` | Dynamic array | `13 arr = [1, 2, 3]` or `list arr = []` |
| `14` | `map` | Key-value associative dictionary | `14 m = {"k": 1}` or `map m = {}` |
| `15+` | `<Class>` | User-defined custom class instances | `15 c = n MyClass()` |

### Builtin System Functions (z Namespace)

The `z` object is pre-bound and provides core runtime capabilities:

- `z.o(value)`: Print output followed by a newline.
- `z.i()`: Read a line of string input from standard input.
- `z.len(collection)`: Returns element count for strings, lists, or maps.
- `z.range(start, stop, step)`: Generates a list of integers over a range.
- `z.thread(function, arg)`: Dispatches function execution to a background OS thread.
- `z.join_all(threads)`: Blocks until all referenced thread IDs complete.
- `z.time()`: Returns high-precision epoch timestamp in seconds.
- `z.assert(condition, message)`: Validates assertion, throwing an exception on failure.

---

## Language Tour and Code Examples

### Variables and Type Inference

```alphabet
#alphabet<en>
// Named types
int score = 95
str player = "Abebe"
bool active = true
list items = ["hammer", "shield", "potion"]
map stats = {"attack": 45, "defense": 30}

// Numeric type IDs
5 count = 10
12 title = "Level 1"

// String interpolation
str welcome = f"Player {player} has score: {score}"
z.o(welcome)
```

### Control Flow: If, Loops, and For-Each

```alphabet
#alphabet<en>
// Conditionals
i (score >= 90) {
    z.o("Grade: A")
} e i (score >= 80) {
    z.o("Grade: B")
} e {
    z.o("Keep practicing")
}

// While loop
5 i = 0
l (i < 3) {
    z.o(f"Counter: {i}")
    i = i + 1
}

// For-each loop over collections
l (item : items) {
    z.o("Inventory item: " + item)
}
```

### Pattern Matching

```alphabet
#alphabet<en>
5 status_code = 404

q (status_code) {
    200: z.o("Success")
    400: z.o("Bad Request")
    404: z.o("Not Found")
    500: z.o("Internal Server Error")
    default: {
        z.o("Unhandled status code: " + status_code)
    }
}
```

### Functions, Closures, and Higher-Order Logic

```alphabet
#alphabet<en>
// Typed function declaration
m 5 multiply(5 a, 5 b) {
    r a * b
}

// Higher-order function receiving a closure
m 13 apply_operation(13 numbers, m op) {
    13 result = []
    l (num : numbers) {
        result.push(op(num))
    }
    r result
}

13 data = [1, 2, 3, 4]
13 doubled = apply_operation(data, m (x) { r x * 2 })
z.o(doubled) // [2, 4, 6, 8]
```

### Object-Oriented Programming and Inheritance

```alphabet
#alphabet<en>
c Animal {
    v 12 name

    Animal(12 n) {
        this.name = n
    }

    v m 12 speak() {
        r "Some generic sound"
    }
}

c Dog : Animal {
    v 12 breed

    Dog(12 n, 12 b) {
        super(n)
        this.breed = b
    }

    v m 12 speak() {
        r "Woof! My name is " + this.name
    }
}

Dog d = n Dog("Rex", "Shepherd")
z.o(d.speak())
```

### Exception Handling

```alphabet
#alphabet<en>
t {
    5 divisor = 0
    i (divisor == 0) {
        z.assert(false, "Division by zero detected")
    }
} h (12 error_message) {
    z.o("Handled exception gracefully: " + error_message)
}
```

### Concurrency and Multithreading

```alphabet
#alphabet<en>
m void background_worker(5 worker_id) {
    z.o(f"Worker {worker_id} started processing")
}

5 thread_1 = z.thread(background_worker, 1)
5 thread_2 = z.thread(background_worker, 2)

// Wait for all threads to complete
z.join_all([thread_1, thread_2])
z.o("All workers finished execution.")
```

---

## Standard Library

Alphabet includes 21 built-in standard modules in `stdlib/`, imported via `x "<name>"`:

| Module | Purpose | Selected Functions and Classes |
|---|---|---|
| `math` | Mathematical functions | `sin`, `cos`, `tan`, `sqrt`, `pow`, `abs`, `ceil`, `floor`, `PI`, `E` |
| `math_ext` | Extended numeric algorithms | `gcd`, `lcm`, `factorial`, `mean`, `variance`, `median` |
| `string` | String utility library | `trim`, `to_upper`, `to_lower`, `split`, `join`, `starts_with`, `replace` |
| `string_utils` | String formatters | `pad_left`, `pad_right`, `repeat`, `truncate`, `levenshtein` |
| `list` | List transformation | `map`, `filter`, `reduce`, `find`, `reverse`, `slice`, `flatten` |
| `list_utils` | Sequence operations | `unique`, `chunk`, `difference`, `intersection`, `zip` |
| `collections` | Data structure collections | `Stack`, `Queue`, `Deque`, `Set`, `PriorityQueue` |
| `data_structures` | Dynamic structures | `LinkedList`, `BinaryTree`, `RingBuffer`, `LRUCache` |
| `functional` | Functional programming | `curry`, `compose`, `pipe`, `memoize`, `partial` |
| `json` | JSON serialization | `parse(str)`, `stringify(obj)`, `validate(str)` |
| `io` | File system streams | `read_file`, `write_file`, `append_file`, `exists`, `remove` |
| `os` | Operating system services | `getenv`, `setenv`, `platform`, `arch`, `exec_cmd` |
| `system` | System diagnostics | `memory_usage`, `cpu_count`, `uptime`, `exit` |
| `crypto` | Cryptography & hashing | `sha256`, `md5`, `hmac_sha256`, `random_bytes` |
| `config` | Configuration parser | `parse_ini`, `parse_env`, `load_config` |
| `test` | Unit testing harness | `describe`, `it`, `assert_equal`, `assert_true`, `run_tests` |

---

## Alphabet Forge: Universal Meta-Compiler Platform

Alphabet Forge is built directly into Alphabet. It allows software engineers and educators to construct completely custom programming languages, compilation toolchains, and developer tooling in minutes from a single declarative specification file (`.forge`).

### Architectural Overview

Forge operates across five decoupled layers:
1. **Header Engine:** Manages file identification, version extraction, and syntax striping.
2. **Branding Engine:** Formats terminal headers, ASCII art, and custom ANSI color palettes.
3. **Packrat PEG Parser:** Parses arbitrary grammar rules with memoization and maps directly to native Alphabet AST nodes.
4. **Dual Backend Emitter:** Compiles to Alphabet stack bytecode or emits standalone C99 for native AOT compilation with cross-compilation target triples.
5. **Ecosystem Generator:** Emits VS Code extensions, LSP servers, debuggers, formatters, and browser-based playgrounds.

### The .forge Specification DSL

```forge
language Nova {
    version: "1.0.0"
    extension: ".nv"
    paradigm: "compiled"
    types: "dynamic"
    author: "Language Designer"

    header {
        style: "prefix"
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
        color: "cyan"
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

### Multi-Style Header Engine

Forge supports five distinct header conventions:

- `prefix`: Language marker followed by delimiters (e.g., `#nova<1.0, compiled>`).
- `shebang`: Standard Unix script executable line (e.g., `#!/usr/bin/env zen`).
- `pragma`: Compiler pragma format (e.g., `@lisp(version=1.0)`).
- `alphabet`: Standard Alphabet family dialect (e.g., `#alphabet<nova>`).
- `none`: Zero-header mode. Source files contain pure source code without headers.

### Dual Execution Pipelines: VM and Native AOT

- **Fast Stack Bytecode VM:** Executes source code instantly with zero ahead-of-time compilation delay.
- **AOT Native C99 Transpiler:** Compiles directly into clean C99 and drives system compilers (`gcc` or `clang`). Supports cross-compilation triples such as `x86_64-w64-mingw32` (Windows `.exe`) and `wasm` (WebAssembly).

### White-Labeled Standalone Toolchains

When forging a language with `alphabet forge <spec.forge> -o bin/<lang>`, Forge generates a self-contained launcher:

```bash
alphabet forge nova.forge -o bin/nova
./bin/nova --version
# Output: Nova Language v1.0.0 (compiled)
```

The resulting `./bin/nova` binary is completely white-labeled. It has its own isolated REPL, help system, test runner, and debugger without referencing Alphabet.

### Scaffolding Wizard (forge init)

Scaffold a complete, production-ready language project in seconds:

```bash
alphabet forge init Apex ./apex-lang --style pragma
```

Generated project directory:
```text
apex-lang/
  ├── Apex.forge        (Language specification)
  ├── README.md         (Documentation and usage guide)
  ├── examples/
  │   └── hello.ape     (Starter program)
  └── tests/
      └── test_basic.ape(Unit test suite)
```

### Grammar and Spec Validator (forge --check)

Validate specification syntax, non-terminal rule completeness, reachability, and left-recursion:

```bash
alphabet forge nova.forge --check
# or via the standalone launcher:
./bin/nova check
```

Validates:
- Metadata integrity (language name, semantic versioning, leading dot on extension).
- Token uniqueness and conflict avoidance.
- Non-terminal reference resolution across PEG rules.
- Direct left-recursion detection to prevent infinite parsing loops.
- Reachability analysis identifying orphan rules.

### Self-Contained Distribution Bundler (forge --bundle)

Export a complete, self-contained distribution package ready for end-user deployment:

```bash
alphabet forge nova.forge --bundle ./dist/nova-v1.0.0
# or via the standalone launcher:
./bin/nova bundle ./dist/nova-v1.0.0
```

Generates an archive (`.tar.gz`) containing:
- `bin/<lang>`: Standalone executable.
- `<Lang>.forge`: Canonical language specification.
- `vscode/`: Ready-to-install VS Code extension.
- `docs/`: Dark-mode HTML documentation website.
- `playground/`: Zero-server in-browser Web IDE and JavaScript interpreter (`index.html`).
- `README.md`: End-user getting-started guide.

### Reference Implementations: Nova, Zen, and Lisp

Alphabet includes three fully functional reference implementations:

1. **Nova (`nova.forge`):** Modern compiled language with prefix header `#nova<version>`, C-style block syntax, and static AOT machine compilation.
2. **Zen (`zen.forge`):** Minimalist scripting language with Unix shebang header (`#!/usr/bin/env zen`), expressive keywords, and bytecode execution.
3. **Lisp (`lisp.forge`):** Functional language demonstrating `@pragma` header style (`@lisp(version=1.0)`), prefix keywords, and bytecode execution.

---

## Installation and Building

### Quick Install (Linux and macOS)

```bash
git clone https://github.com/fraol163/alphabet.git
cd alphabet
./install.sh
```

### Windows Installation

```powershell
git clone https://github.com/fraol163/alphabet.git
cd alphabet
.\install.ps1
```

### Building from Source with CMake

**Prerequisites:**
- CMake 3.16 or higher
- C++17 compliant compiler: GCC 9+, Clang 10+, or MSVC 2019+

```bash
cmake -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
```

Run test suite:
```bash
ctest --test-dir build --output-on-failure
```

### Editor Support and VS Code Extension

- **Visual Studio Code:** Install the official extension from the [VS Code Marketplace](https://marketplace.visualstudio.com/items?itemName=fraolteshome.alphabet).
- **Other Editors (Vim, Neovim, Emacs, Helix):** Configure your editor's LSP client to execute `alphabet --lsp` over standard I/O.
- **TextMate Grammar:** Available at `editors/vscode-alphabet/syntaxes/alphabet.tmLanguage.json`.

### Verifying Installation

```bash
alphabet --version
# Output: Alphabet 2.3.6 (Native C++)
```

---

## Command Line Interface Reference

### Alphabet CLI Commands

```bash
alphabet run <file.abc> [args]    # Execute program with optional arguments
alphabet watch <file.abc>         # Re-run file automatically on changes
alphabet --repl                   # Launch interactive REPL session
alphabet --lsp                    # Start Language Server Protocol daemon
alphabet --debug <file.abc>       # Step-by-step interactive terminal debugger
alphabet --dump-bytecode <file>   # Disassemble and inspect compiled opcodes
alphabet -c -o <out.bin> <file>   # Compile directly to bytecode binary
alphabet fmt <file.abc>           # Format Alphabet source code
alphabet lint <file.abc>          # Lint source code for static warnings
alphabet test [dir]               # Execute test suite
alphabet doc [topic]              # View interactive documentation
alphabet bench                    # Run internal virtual machine benchmarks
alphabet update                   # Self-update to latest release
```

### Alphabet Forge Subcommands

```bash
alphabet forge init <name> [dir]  # Scaffold a new custom language project
alphabet forge <spec.forge> -o b  # Forge standalone toolchain binary
alphabet forge <spec.forge> --check Validate spec and grammar integrity
alphabet forge <spec.forge> --bundle Export complete distribution bundle
alphabet forge <spec.forge> --export-vscode Export ready-to-use VS Code extension
alphabet forge <spec.forge> --doc Generate HTML documentation site
alphabet forge <spec.forge> --playground Export client-side Web Playground
alphabet forge --repl --spec <s>  # Launch custom language REPL
```

### Interactive REPL Reference

Start the REPL by running `alphabet --repl`.

- `.vars`: Display all defined variables and active types.
- `.keywords`: Show keyword mappings for current language header.
- `.clear`: Clear screen and history.
- `quit` / `exit`: Exit the REPL session.

### Interactive Step Debugger Reference

Start the debugger with `alphabet --debug <file.abc>`:

- `s` / `step`: Step to next line.
- `si`: Step single bytecode instruction.
- `c` / `continue`: Resume execution until next breakpoint.
- `b <line>`: Add line breakpoint.
- `db <line>`: Delete line breakpoint.
- `bl`: List all active breakpoints.
- `p <var>`: Print value of variable.
- `l` / `locals`: Display all active local variables.
- `g` / `globals`: Display all active global variables.
- `bt` / `stack`: Inspect call stack trace.
- `q` / `quit`: Exit debugger.

---

## Testing and Verification

Alphabet maintains a zero-regression test suite:

- **Framework:** Catch2 and CMake CTest.
- **Results:** **35 / 35 test suites passing (100%)** across:
  - Lexer and custom tokenizer matrices
  - Parser AST generation and grammar rules
  - Virtual machine opcodes and garbage collection lifecycle
  - End-to-end golden tests across all 5 language dialects
  - Alphabet Forge meta-compiler, validator, and packrat parsers
  - Edge case stress tests (recursion, float precision, memory allocation)

Execute the verification suite:
```bash
ctest --test-dir build --output-on-failure
```

---

## What is New in v2.3.6

Alphabet **v2.3.6** is a major milestone release combining comprehensive platform audits with the debut of Alphabet Forge:

1. **Alphabet Forge Engine:**
   - Universal meta-compiler and language generator (`alphabet forge`).
   - Declarative `.forge` grammar and token mapping DSL.
   - Dual execution pipelines: Stack VM and Native AOT C99 transpiler with cross-compilation targets.
   - White-labeled launcher generator producing branded `./bin/<lang>` binaries.
   - Complete ecosystem scaffolding: VS Code extensions, LSP servers, interactive debuggers (CLI & DAP), documentation sites, and zero-server in-browser playgrounds.
   - Project scaffolding wizard (`forge init`) and distribution bundle packaging (`forge --bundle`).
   - Reference implementations included: `nova.forge`, `zen.forge`, and `lisp.forge`.
2. **Master Compiler and Runtime Hardening:**
   - **Type Inference Engine Overhaul:** Fixed fallback logic in expression type resolution; full typing support for F-strings, binary operations, variable lookups, and method return contracts.
   - **Pattern Matching default Arm:** Full support for `default:` keyword across all language dialects without misinterpreting case keys.
   - **Stdlib Resolution Fallback:** Unqualified imports (`x "test"`) automatically resolve against `<source_dir>/stdlib/` and `<cwd>/stdlib/` without requiring manual `ALPHABET_PATH` configuration.
   - **Abstract Class Enforcement:** Instantiation of abstract classes strictly blocked at bytecode execution.
   - **Visibility and Modifier Parsing:** Fixed multilingual class modifier loop parsing for German, Spanish, French, and Amharic.
   - **String Arithmetic:** Corrected string repetition (`"ab" * 3`) width promotion and boundary behavior.
   - **Memory Safety:** Mitigated buffer invalidation in WASM playground exports using rotating double buffers.
3. **Packaging and Distribution:**
   - Corrected official repository endpoints across Homebrew (`alphabet.rb`), Arch Linux AUR (`PKGBUILD`), and Canonical Snap (`snapcraft.yaml`).
   - Added full shell autocompletions for Bash, Zsh, and Fish.

---

## Contributing

Contributions from developers of all backgrounds are welcomed:

1. Fork the repository: `https://github.com/fraol163/alphabet`
2. Create a feature branch: `git checkout -b feature/amazing-feature`
3. Verify test pass rate: `ctest --test-dir build`
4. Commit your changes: `git commit -m 'feat: add amazing feature'`
5. Push to branch: `git push origin feature/amazing-feature`
6. Open a Pull Request.

---

## License

Alphabet is released under the permissive [MIT License](LICENSE.txt). You are free to use, modify, distribute, embed, and build commercial products upon Alphabet without restriction.

---

## Contact

- **Author:** Fraol Teshome
- **Email:** fraolteshome444@gmail.com
- **GitHub:** [https://github.com/fraol163/alphabet](https://github.com/fraol163/alphabet)
- **Issues:** [https://github.com/fraol163/alphabet/issues](https://github.com/fraol163/alphabet/issues)
- **Discussions:** [https://github.com/fraol163/alphabet/discussions](https://github.com/fraol163/alphabet/discussions)

**Engineered in C++17 for universal access and expressive computing.**
