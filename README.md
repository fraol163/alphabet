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
# Alphabet Programming Language V2.3.5

**Learn to think. Code in your language.**

[![Build Status](https://github.com/fraol163/alphabet/actions/workflows/ci.yml/badge.svg)](https://github.com/fraol163/alphabet/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](LICENSE.txt)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)](#installation)

---

## Table of Contents

- [What is Alphabet?](#what-is-alphabet)
  - [Why Alphabet?](#why-alphabet)
  - [Key Features](#key-features)
  - [Who Is Alphabet For?](#who-is-alphabet-for)
- [Installation](#installation)
- [Hello World](#hello-world)
- [Language Comparison](#language-comparison)
- [Language Features](#language-features)
  - [Variables and Types](#variables-and-types)
  - [Control Flow](#control-flow)
  - [For Loop](#for-loop)
  - [Break and Continue](#break-and-continue)
  - [Functions](#functions)
  - [Classes](#classes)
  - [Pattern Matching](#pattern-matching)
  - [Exception Handling](#exception-handling)
  - [Import Modules](#import-modules)
  - [String Escapes and Concatenation](#string-escapes-and-concatenation)
  - [String Interpolation](#string-interpolation)
  - [FFI - Call Native C Functions](#ffi---call-native-c-functions)
  - [Built-In Functions](#built-in-functions)
  - [Multilingual Keywords](#multilingual-keywords)
- [Standard Library](#standard-library)
  - [Stdlib Modules](#stdlib-modules)
- [Examples](#examples)
  - [Example Programs](#example-programs)
- [Learning Curriculum](#learning-curriculum)
- [Command Line](#command-line)
  - [CLI Flags](#cli-flags)
  - [CLI Subcommands](#cli-subcommands)
  - [REPL Commands](#repl-commands)
  - [Shebang Scripts](#shebang-scripts)
  - [Environment Variables](#environment-variables)
- [Documentation](#documentation)
  - [Getting Started](#getting-started)
  - [Reference](#reference)
  - [Advanced](#advanced)
  - [Community](#community)
- [Testing](#testing)
- [Project Statistics](#project-statistics)
- [v2.3.4 vs v2.3.5](#v234-vs-v235)
  - [What v2.3.4 Had](#what-v234-had)
  - [What's New in v2.3.5](#whats-new-in-v235)
- [What Problem Does Alphabet Solve?](#what-problem-does-alphabet-solve)
- [Contributing](#contributing)
- [License](#license)
- [Contact](#contact)

---

## What is Alphabet?

Alphabet is a **programming language that teaches you to think like a programmer** — in your own language. Write code in English, Amharic, Spanish, French, or German. Learn concepts that transfer to Python, Java, JavaScript, and beyond. Unlike most programming languages that force you to learn English AND programming at the same time, Alphabet removes that barrier. You learn concepts first, syntax second. The language uses only 26 keyword entries (with aliases) per language mapped to 22 single-letter symbols, making it one of the simplest languages to pick up. It runs on a bytecode virtual machine with compiled execution, supports compile-time type checking, and includes a built-in REPL that can teach you programming concepts interactively. Whether you are a student learning your first loop, an educator teaching algorithms, or a developer prototyping something that needs to run anywhere, Alphabet gives you a clean, minimal syntax that transfers directly to production languages.

### Why Alphabet?

Most programming languages force you to learn English AND programming at the same time. Alphabet removes that barrier. Learn concepts first, syntax second.

| What you learn | Alphabet | Transfers to |
|----------------|----------|--------------|
| `variable = labeled storage` | `int x = 10` | Python, Java, JS, Go |
| `condition = decision` | `i (x > 0) { }` | Every language |
| `loop = repeat` | `l (cond) { }` | Every language |
| `function = recipe` | `void f() { }` | Every language |
| `class = blueprint` | `c Car { }` | Java, C++, Python |

### Key Features

- **Multilingual** - Write code in English, Amharic, Spanish, French, or German
- **Named Types** - `int x = 10`, `str name = "hello"` — readable, transferable syntax
- **Type Inference** - let the compiler figure it out
- **Concept-First** - REPL teaches concepts before syntax
- **Simple I/O** - `o("hello")` to print, `i()` to read
- **For-Each Loop** - `l (item : list) { o(item) }` — iterate collections naturally
- **Range Expressions** - `1..10` creates a list, works in loops and assignments
- **Raw Strings** - `r"no\escapes"` — backslashes are literal
- **Bytecode VM** - Stack-based interpreter with compiled execution
- **Type System** - Compile-time type checking with named type keywords
- **LSP Support** - Works with VS Code and other editors
- **String Interpolation** - f-strings: `f"Hello {name}"`
- **FFI** - Call native C functions from Alphabet code
- **Cross-Platform** - Works on Windows, Linux, and macOS
- **Built-in Debugger** - Breakpoints, step, locals, globals, stack trace
- **REPL** - Interactive mode with concept explanations and error recovery
- **Standard Library** - 21 modules: Math, I/O, String, List, JSON, Crypto, Config, Test, and more
- **Embedding API** - Embed Alphabet in C++ applications
- **Voice Input** - Speak code in any of 5 languages (Vosk + Whisper)
- **NL-to-Code** - Natural language to code conversion
- **Threading** - `z.thread()`, `z.join_all()` for concurrent execution

### Who Is Alphabet For?

**For Non-English Speakers:** Learn programming concepts in your native language. No need to learn English first.

**For Students:** Focus on programming concepts — variables, loops, functions, classes — not memorizing syntax.

**For Educators:** Teach logic and algorithms with concept explanations built into the REPL.

**For Anyone Moving to Python/Java/JS:** Every concept you learn in Alphabet transfers directly. `int x = 10` in Alphabet is `int x = 10` in Java.

---

## Language Comparison

Alphabet occupies a unique position in the programming language landscape. While C and Zig are powerful systems languages with steep learning curves, and Python is a versatile scripting language with English-only syntax, Alphabet focuses specifically on education and accessibility. With only 26 keyword entries per language, it has the smallest keyword set of any mainstream-style language, making it ideal for beginners. The numeric type system (using `5` for int, `12` for string, etc.) is unconventional but teaches the concept of type IDs that exist in every compiled language. The for-each loop, pattern matching, and exception handling features mirror what you will find in production languages, so skills transfer directly. Alphabet is not trying to replace Python or Java — it is designed to be the language you learn first, so that every other language becomes easier to pick up.

| Feature | C | Python | Zig | **Alphabet** |
|---------|---|--------|-----|--------------|
| Keywords | 32 | 35 | 43 | **26 entries** |
| Type System | Manual | Dynamic | Static | **Numeric IDs** |
| Learning Curve | Steep | Medium | Medium | **Flat** |
| Best For | Systems | Scripts | Systems | **Education** |

---

## Command Line

The Alphabet command line provides 13 subcommands plus core flags that cover every aspect of the development workflow. You can run programs directly, start an interactive REPL for experimentation, launch an LSP server for editor integration, debug programs with breakpoints, compile without running, inspect bytecode, sandbox execution for security, lint code for warnings, run tests, manage projects, access documentation for all builtins, run benchmarks, explore examples, take an interactive tour, set up voice input, and self-update to the latest version. The CLI is designed to be discoverable — `alphabet --help` shows all options, and each subcommand has its own help text. Programs can also be run as executable scripts using shebang lines on Linux and macOS.

```bash
# Run a program
alphabet program.abc
alphabet run program.abc          # explicit subcommand

# Run with arguments
alphabet run program.abc arg1 arg2

# Interactive REPL
alphabet --repl

# Learning curriculum
alphabet examples                 # list all examples
alphabet tour                     # interactive tour

# Create new project
alphabet init myproject

# Run project tests
alphabet test

# LSP server for editor integration
alphabet --lsp

# Debug with breakpoints
alphabet --debug program.abc

# Inspect compiled bytecode
alphabet --dump-bytecode program.abc

# Sandbox mode (block FFI and file access)
alphabet --sandbox program.abc

# Self-update to latest version
alphabet update

# Show version
alphabet --version
```

### CLI Flags

| Flag | Description |
|------|-------------|
| `--version`, `-v` | Show version information |
| `--help`, `-h` | Show help message |
| `--compile`, `-c` | Compile only, don't run |
| `--output`, `-o` | Output file for compiled bytecode |
| `--repl` | Start interactive REPL |
| `--lsp` | Start Language Server Protocol server |
| `--debug` | Run in debug mode (breakpoints) |
| `--sandbox` | Sandbox mode: block FFI and file access |
| `--dump-bytecode` | Print compiled bytecode and exit |

### CLI Subcommands

| Subcommand | Description |
|------------|-------------|
| `run <file>` | Run a program |
| `init <name>` | Create new project structure |
| `test [dir]` | Run test files |
| `info` | Show project info |
| `doc <name>` | Show builtin documentation |
| `bench` | Run VM benchmark suite |
| `examples` | List available examples |
| `tour` | Interactive language tour |
| `lint <file>` | Lint a file for warnings |
| `pkg` | Package manager (install, list, search) |
| `setup-voice` | Install voice dependencies |
| `voice-tutorial` | Learn voice input |
| `update` | Self-update to latest version |

### REPL Commands

| Command | Description |
|---------|-------------|
| `q`, `quit`, `exit` | Exit REPL |
| `help` | Show commands |
| `clear` | Clear screen |
| `clear history` | Clear command history |
| `clear all` | Clear screen + history |
| `reset` | Clear everything |
| `vars` | Show defined variables |
| `keywords` | Show keywords for current language |
| `history` | Show command history |
| `!!` | Repeat last command |
| `trace` | Show trace mode status |
| `trace on` | Enable execution trace |
| `trace off` | Disable execution trace |
| `trace slow` | Watch each step happen gradually |
| `trace fast` | Instant results (default) |
| `lang` | Show current language |
| `builtins` | List all builtins by category |
| `voice` | Voice-to-text input (checks deps first) |
| `reload` | Clear variables, keep code |

### Shebang Scripts

Make Alphabet programs executable on Linux/macOS:

```bash
# my_script.abc
#!/usr/bin/env alphabet
#alphabet<en>
o("Hello from a script!")
```

```bash
chmod +x my_script.abc
./my_script.abc
```

### Environment Variables

| Variable | Description |
|----------|-------------|
| `ALPHABET_PATH` | Colon-separated directories to search for imports |

---

## Documentation

Alphabet provides comprehensive documentation organized into four categories to serve every learning style and experience level. The Getting Started section includes a beginner guide that walks you through your first program in 10 minutes, a step-by-step tutorial with progressively complex examples, and a complete guide that covers every feature in depth. The Reference section provides a printable quick reference card, a detailed language reference covering types, operators, and syntax, and an installation guide for every platform. The Advanced section covers performance benchmarks, deep dives into FFI and LSP integration, and the technical architecture. The Community section includes contributing guidelines and the project roadmap showing what features are planned for future releases.

### Getting Started

| Guide | Description |
|-------|-------------|
| [Getting Started](docs/GETTING_STARTED.md) | Beginner guide (10 min) |
| [Tutorial](docs/TUTORIAL.md) | Step-by-step examples |
| [Complete Guide](docs/COMPLETE_GUIDE.md) | Full language tutorial |

### Reference

| Guide | Description |
|-------|-------------|
| [Quick Reference](QUICK_REFERENCE.md) | Printable cheat sheet |
| [Language Reference](docs/REFERENCE.md) | Keywords, types, operators |
| [Installation](docs/INSTALLATION.md) | Setup guide |

### Advanced

| Guide | Description |
|-------|-------------|
| [Benchmarks](docs/BENCHMARKS.md) | Performance data |
| [Advanced Features](docs/ADVANCED.md) | FFI, LSP, architecture |
| [Architecture](docs/PRESENTATION.md) | Technical overview |

### Community

| Guide | Description |
|-------|-------------|
| [Contributing](CONTRIBUTING.md) | How to contribute |
| [Roadmap](docs/ROADMAP.md) | Future development |

---

## Testing

Alphabet includes a comprehensive test suite that verifies every component of the language works correctly. The test suite runs 34 ctest unit tests covering the lexer, parser, compiler, and virtual machine, plus 257 test files that verify all 5 keyword languages (English, Amharic, Spanish, French, German) produce correct output. The tests cover variables, control flow, loops, functions, classes, inheritance, static methods, constructors, this keyword, string operations, list operations, map operations, pattern matching, exception handling, closures, and more. All tests run automatically with CMake and CTest, and the CI pipeline runs them on every commit to catch regressions. The test results are displayed with clear pass/fail indicators and detailed error messages when something breaks.

```bash
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

---

## Project Statistics

| Metric | Value |
|--------|-------|
| Version | v2.3.5 |
| Tests | 34 ctest + 257 test files |
| Builtins | 74 functions |
| Stdlib Modules | 21 |
| Examples | 38 programs |
| CLI Subcommands | 13 |
| Languages | 5 (en/am/es/fr/de) |
| Keywords | 26 entries per language (22 unique symbols) |
| Opcodes | 45 |
| Token Types | 45 |
| Source Files | 34 (.cpp + .h) |
| LOC | 13,827 C++ |
| Optimization | Constant folding pass |
| Learning Lessons | 10 (with exercise + solution) |

---

## Standard Library

The Alphabet standard library provides 21 reusable modules covering math, I/O, strings, lists, JSON, crypto, configuration, testing, and more. Every module is implemented in Alphabet itself (`.abc` files) and can be imported with `x <module>`. The complete list is below.

### Stdlib Modules

| Module | Description |
|--------|-------------|
| `math` | Basic math: sqrt, pow, abs, min, max, floor, ceil, round |
| `math_ext` | Extended math: trigonometric, logarithmic, statistics |
| `string` | String operations: upper, lower, find, replace, split, trim |
| `string_utils` | Advanced string utilities: padding, alignment, templates |
| `list` | List operations: map, filter, reduce, sort, reverse |
| `list_utils` | List utilities: chunk, flatten, zip, enumerate, unique |
| `collections` | Collection helpers: stack, queue, sets |
| `data_structures` | Data structures: linked lists, trees, graphs |
| `functional` | Functional programming: compose, curry, partial application |
| `json` | JSON parsing and generation |
| `io` | File I/O: read, write, append, exists |
| `os` | OS operations: env, args, exec, system calls |
| `system` | System info: timestamp, env vars, process control |
| `datetime` | Date and time formatting |
| `random` | Random number generation: rand, randint, choice |
| `crypto` | Cryptographic operations: hash, hmac |
| `config` | Configuration file parsing (TOML-style) |
| `validate` | Input validation helpers |
| `assert` | Assertion helpers for testing |
| `test` | Test framework: describe, it, expect |
| `testing` | Extended testing utilities |

Import a module with `x <module>`:

```alphabet
#alphabet<en>
x math
5 r = math.sqrt(16)
z.o(r)

x json
m data = {"name": "Alice", "age": 30}
z.o(json.stringify(data))
```

---

## Examples

Alphabet ships with 38 example programs covering every major feature. Browse them locally or copy them as starting points for your own projects.

### Example Programs

| File | Description |
|------|-------------|
| `01_hello.abc` | First program |
| `02_collections.abc` | Lists and maps |
| `03_exceptions.abc` | Try/handle blocks |
| `04_statics.abc` | Static fields and methods |
| `amharic.abc` | Amharic keywords demo |
| `ascii_art.abc` | ASCII Art Generator |
| `benchmarking.abc` | Timing and performance measurement |
| `caesar_cipher.abc` | Caesar cipher encryption |
| `calculator.abc` | Calculator example |
| `classes.abc` | Class-based OOP example |
| `cli_tool.abc` | CLI Tool Example |
| `comprehensive.abc` | Comprehensive feature test |
| `data_processing.abc` | Data processing pipeline |
| `design_patterns.abc` | Singleton, Observer, Strategy patterns |
| `error_handling.abc` | Error handling patterns |
| `fibonacci.abc` | Fibonacci sequence calculator |
| `file_io.abc` | File I/O example |
| `functional.abc` | Functional programming patterns |
| `guessing_game.abc` | Number guessing game |
| `hello_shebang.abc` | Hello world with shebang line |
| `json.abc` | JSON parsing and generation |
| `lib_greeter.abc` | Greeter class library example |
| `lib_math.abc` | Math library example |
| `matrix.abc` | Matrix operations (create, add, multiply, print) |
| `networking.abc` | Networking Utilities |
| `pattern_matching.abc` | Match/case pattern matching |
| `recursion.abc` | Tower of Hanoi, binary search, merge sort |
| `sorting.abc` | Bubble sort and selection sort |
| `state_machine.abc` | State machine implementation |
| `text_adventure.abc` | Mini text adventure game |
| `unit_testing.abc` | Unit testing framework usage |
| `web_api.abc` | HTTP requests, JSON, error handling |
| `test_break.abc` | Break statement tests |
| `test_continue.abc` | Continue statement tests |
| `test_escapes.abc` | String escape sequence tests |
| `test_import.abc` | Module import tests |
| `test_import_class.abc` | Class import tests |
| `test_stdlib.abc` | Standard library smoke test |

Run any example with `alphabet examples/<file>` or browse them all with `alphabet examples`.

---

## Learning Curriculum

Alphabet ships with a 10-lesson interactive curriculum. Each lesson has an `exercise.abc` (with blanks to fill in) and a `solution.abc` (the complete working version). Walk through them in order for a structured introduction to the language.

| # | Lesson | Topics |
|---|--------|--------|
| 01 | `01_hello_world` | Print, basic syntax, comments |
| 02 | `02_variables` | Variable declaration, named types, type inference |
| 03 | `03_basic_math` | Arithmetic, operators, math functions |
| 04 | `04_conditionals` | If/else, comparison operators, boolean logic |
| 05 | `05_loops` | While loops, for loops, break, continue |
| 06 | `06_functions` | Function declaration, parameters, return values |
| 07 | `07_lists` | List creation, indexing, iteration, list operations |
| 08 | `08_classes` | Class declaration, fields, methods, constructors |
| 09 | `09_error_handling` | Try/handle, exception types, error recovery |
| 10 | `10_project_calculator` | Capstone: build a calculator combining all concepts |

Each lesson has an exercise (with `___` placeholders) and a solution. Use the REPL or your editor to fill in the blanks, then run with `alphabet learning/<NN_topic>/solution.abc` to see the expected output.

---

## v2.3.4 vs v2.3.5

This section compares what was available in v2.3.4 and what was added or fixed in v2.3.5. v2.3.5 was released June 2026 and is the current version. v2.3.4 was the previous release.

### What v2.3.4 Had

v2.3.4 was the first feature-complete release with a working compiler, VM, and toolchain. It included:

- **Core Language**: Variables, types, control flow (if/else, loops), functions, classes, inheritance, abstract classes, interfaces
- **Advanced Features**: Closures, lambdas, for-each loops, range expressions, ternary operator, null-safe operator, pattern matching, exception handling
- **OOP**: Public/private visibility, static methods, default parameters, labeled break/continue, operator overloading
- **VM**: 45 opcodes, stack-based execution, garbage collection, bytecode serialization
- **REPL**: Interactive mode with history, multi-line input, language switching
- **Tooling**: LSP server, debugger with breakpoints, FFI for native C calls, linter, formatter
- **Standard Library**: Math, I/O, String, List modules
- **Testing**: 34 ctest unit tests, language tests for all 5 languages
- **Documentation**: Grammar spec, API docs, examples

### What's New in v2.3.5

v2.3.5 is a major release that expanded the standard library, added voice input, browser playground support, threading, improved error messages, fixed several compiler bugs, and added the interactive learning curriculum. The summary table below shows the before/after counts, followed by the specific features that are genuinely new (not carried over from v2.3.4).

#### Genuinely New in v2.3.5

| Category | v2.3.4 | v2.3.5 | Added |
|----------|--------|--------|-------|
| CLI Subcommands | 0 | 13 | +13 |
| REPL Commands | 12 | 19 | +7 |
| Stdlib Modules | 4 | 21 | +17 |
| Test Files | 34 ctest | 34 ctest + 257 | +257 |
| Examples | 0 | 38 | +38 |
| Learning Lessons | 0 | 10 | +10 |
| LOC | ~12,000 | 13,827 | +1,800 |
| Source Files | 30 | 34 | +4 |
| Builtins | ~60 | 74 | +14 |

#### Features New in v2.3.5 (Not in v2.3.4)

1. **super()** - Call parent class constructors from child classes.
2. **Trace Mode** - REPL shows 4-phase execution (tokenize, parse, compile, execute) with stack depth. Includes `trace slow` for step-by-step animation.
3. **Package Manager** - `alphabet pkg list/search/install` for stdlib modules.
4. **Improved Error Messages** - Hint system explains keyword conflicts (`'i' is the 'if' keyword. Use a different variable name.`). Works for all 26 keywords.
5. **Embedding API** - `alphabet_embed.h` lets C++ apps embed the Alphabet VM.
6. **Project Management** - `alphabet init myproject` scaffolds a project, `alphabet info` shows project metadata.
7. **Voice Input** - Speak code in 5 languages (Vosk for en/es/fr/de offline, Whisper for Amharic offline). Setup: `alphabet setup-voice`.
8. **Browser Playground** - Web playground at `playground/index.html` with 7 examples and 5 language selector.
9. **Trace Slow Mode** - `trace slow` makes each step appear with a delay for learning how compilation works.
10. **17 New Stdlib Modules** - math_ext, string_utils, list_utils, collections, data_structures, functional, json, io, os, system, datetime, random, crypto, config, validate, assert, test, testing.
11. **Bug Fixes**:
    - `continue` in for-loops no longer causes infinite loops
    - Static methods now work correctly
    - Constructor dispatch looks for class name, not `init`
    - Nested for-each variable name conflicts fixed with unique `__feN` names
    - Abstract class parsing (`a c ClassName`) now works
    - `this.field` type inference now correct
12. **CLI Self-Update** - `alphabet update` and `alphabet update --force` to reinstall current version.
13. **REPL `reload`** - Clears variables but keeps accumulated code.
14. **REPL `voice`** - Checks voice dependencies before starting.
15. **REPL `builtins`** - Lists all builtins by category.
16. **`q` is exclusively the `match` keyword** - Use `quit`/`exit` to leave REPL.
17. **38 Working Examples** - Comprehensive coverage of every language feature.
18. **10-Lesson Learning Curriculum** - Structured interactive lessons with exercises and solutions.

#### Features Already in v2.3.4 (Not New in v2.3.5)

To avoid confusion, these features were already available in v2.3.4 and were not added in v2.3.5:

- Closures and lambdas
- `map`, `filter`, `reduce` higher-order functions
- Ternary operator (`x > 5 ? "big" : "small"`)
- For-each loops (`l (item : list) { ... }`)
- Range expressions (`1..10`)
- Threading support (`z.thread`, `z.join_all`)
- Null-safe operator
- Public/private visibility
- Static methods
- FFI for native C calls
- LSP server
- Debugger with breakpoints
- Pattern matching
- Exception handling
- Abstract classes and interfaces
- Operator overloading
- Default parameters
- Labeled break/continue
- 45-opcode VM
- Garbage collection
- Bytecode serialization
- Constant folding

---

## What Problem Does Alphabet Solve?

Alphabet addresses four fundamental problems that make programming harder than it needs to be. First, **education overload**: beginners struggle with syntax before understanding concepts, and Alphabet's minimal keyword design removes that barrier. Second, **rapid prototyping**: quick experiments without boilerplate code, letting you focus on ideas rather than ceremony. Third, **multilingual programming**: write code in your native language so you can learn logic without fighting English syntax. Fourth, **tooling integration**: built-in LSP server, debugger, and REPL work with any editor supporting the Language Server Protocol. These four problems affect millions of learners worldwide, and Alphabet solves them all with a single, minimal language that teaches concepts that transfer to every other programming language you will ever use.

### 1. Education Overload

Beginners struggle with syntax before understanding concepts. Alphabet removes this barrier with only 26 keyword entries per language.

### 2. Rapid Prototyping

Quick experiments without boilerplate. Compare:

**Python:**
```python
def factorial(n):
    if n <= 1:
        return 1
    return n * factorial(n - 1)
```

**Alphabet:**
```alphabet
#alphabet<en>
m 5 factorial(5 num) {
  i (num <= 1) { r 1 }
  r num * factorial(num - 1)
}
```

### 3. Multilingual Programming

Write code in your native language. Alphabet supports English, Amharic, Spanish, French, and German keywords.

### 4. Tooling Integration

Built-in LSP server works with VS Code, Vim, Emacs, and any editor supporting the Language Server Protocol.

---

## Contributing

Alphabet is an open source project that welcomes contributions from developers of all skill levels. Whether you want to fix a bug, add a feature, improve documentation, or report an issue, your help is appreciated. The project uses GitHub for collaboration — you can report bugs through GitHub Issues, discuss ideas in GitHub Discussions, and submit code changes through pull requests. The CONTRIBUTING.md file provides detailed guidelines including project structure, coding standards, testing requirements, and the review process. First-time contributors are especially welcome, and good first issues are labeled to help you find accessible starting points.

- Report bugs: [GitHub Issues](https://github.com/fraol163/alphabet/issues)
- Discuss: [GitHub Discussions](https://github.com/fraol163/alphabet/discussions)

---

## License

Alphabet is released under the MIT License, one of the most permissive open source licenses available. This means you are free to use, modify, and distribute the language for any purpose — personal, educational, or commercial — without restriction. You can embed Alphabet in your own projects, create derivative works, and even sell products that include it. The only requirement is that you include the original copyright notice and license text in any distribution. The full license text is available in the [LICENSE.txt](LICENSE.txt) file in the project root.

---

## Contact

Alphabet is created and maintained by Fraol Teshome, a developer passionate about making programming education accessible to everyone regardless of their native language. You can reach out via email (fraolteshome444@gmail.com) for questions, feedback, collaboration ideas, or just to say hello. The project is hosted on GitHub where you can follow development, report issues, and join discussions. If you are using Alphabet in education, research, or a personal project, I would love to hear about your experience — your feedback helps shape the future of the language.

**Built with C++17**
