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
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)](#installation)

---

## Table of Contents

- [What is Alphabet?](#what-is-alphabet)
- [Installation](#installation)
- [Hello World](#hello-world)
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
- [Command Line](#command-line)
- [Documentation](#documentation)
- [Testing](#testing)
- [v2.3.4 vs v2.3.5](#v234-vs-v235)
- [What Problem Does Alphabet Solve?](#what-problem-does-alphabet-solve)
- [Contributing](#contributing)
- [License](#license)

---

## What is Alphabet?

Alphabet is a **programming language that teaches you to think like a programmer** —> in your own language. Write code in English, Amharic, Spanish, French, or German. Learn concepts that transfer to Python, Java, JavaScript, and beyond. Unlike most programming languages that force you to learn English AND programming at the same time, Alphabet removes that barrier. You learn concepts first, syntax second. The language uses only 19 keywords (compared to 32+ in C or 35+ in Python), making it one of the simplest languages to pick up. It runs on a bytecode virtual machine with compiled execution, supports compile-time type checking, and includes a built-in REPL that can teach you programming concepts interactively. Whether you are a student learning your first loop, an educator teaching algorithms, or a developer prototype that needs to run anywhere, Alphabet gives you a clean, minimal syntax that transfers directly to production languages.

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
- **Type Inference** - `let x = 10`, `val name = "hello"` — let the compiler figure it out
- **Concept-First** - `explain variable` in REPL teaches concepts before syntax
- **Simple I/O** - `o("hello")` to print, `input()` to read — no namespace needed
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
- **Threading** - z.thread(), z.join(), z.lock() for concurrent execution

### Who Is Alphabet For?

**For Non-English Speakers:** Learn programming concepts in your native language. No need to learn English first.

**For Students:** Focus on programming concepts — variables, loops, functions, classes — not memorizing syntax.

**For Educators:** Teach logic and algorithms with concept explanations built into the REPL. `explain variable` teaches the concept before the code.

**For Anyone Moving to Python/Java/JS:** Every concept you learn in Alphabet transfers directly. `int x = 10` in Alphabet is `int x = 10` in Java.


## Language Comparison

Alphabet occupies a unique position in the programming language landscape. While C and Zig are powerful systems languages with steep learning curves, and Python is a versatile scripting language with English-only syntax, Alphabet focuses specifically on education and accessibility. With only 19 keywords, it has the smallest keyword set of any mainstream-style language, making it ideal for beginners. The numeric type system (using `5` for int, `12` for string, etc.) is unconventional but teaches the concept of type IDs that exist in every compiled language. The for-each loop, pattern matching, and exception handling features mirror what you will find in production languages, so skills transfer directly. Alphabet is not trying to replace Python or Java —> it is designed to be the language you learn first, so that every other language becomes easier to pick up.

| Feature | C | Python | Zig | **Alphabet** |
|---------|---|--------|-----|--------------|
| Keywords | 32 | 35 | 43 | **19** |
| Type System | Manual | Dynamic | Static | **Numeric IDs** |
| Learning Curve | Steep | Medium | Medium | **Flat** |
| Best For | Systems | Scripts | Systems | **Education** |

---

## Command Line

The Alphabet command line provides 22 subcommands that cover every aspect of the development workflow. You can run programs directly, start an interactive REPL for experimentation, launch an LSP server for editor integration, debug programs with breakpoints, compile without running, inspect bytecode, sandbox execution for security, lint code for warnings, run tests, manage projects, access documentation for all 82 builtins, run benchmarks, explore examples, take an interactive tour, set up voice input, and self-update to the latest version. The CLI is designed to be discoverable —> `alphabet --help` shows all options, and each subcommand has its own help text. Programs can also be run as executable scripts using shebang lines on Linux and macOS.

```bash
# Run a program
alphabet program.abc
alphabet run program.abc          # explicit subcommand

# Run with arguments
alphabet run program.abc arg1 arg2

# Interactive REPL
alphabet --repl

# Learning curriculum
alphabet learn                    # list all lessons
alphabet learn 01                 # show lesson 01
alphabet learn 01 --run           # run exercise and check

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
# Show help
alphabet --help
```
```
### Environment Variables

| Variable | Description |
|----------|-------------|
| `ALPHABET_PATH` | Colon-separated directories to search for imports |

---

## Documentation

Alphabet provides comprehensive documentation organized into four categories to serve every learning style and experience level. The Getting Started section includes a beginner guide that walks you through your first program in 10 minutes, a step-by-step tutorial with progressively complex examples, and a complete guide that covers every feature in depth. The Reference section provides a printable quick reference card with all 19 keywords, a detailed language reference covering types, operators, and syntax, and an installation guide for every platform. The Advanced section covers performance benchmarks, deep dives into FFI and LSP integration, and the technical architecture. The Community section includes contributing guidelines and the project roadmap showing what features are planned for future releases.

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

Alphabet includes a comprehensive test suite that verifies every component of the language works correctly. The test suite runs 34 unit tests covering the lexer, parser, compiler, and virtual machine, plus 257 test files that verify all 5 keyword languages (English, Amharic, Spanish, French, German) produce correct output. The tests cover variables, control flow, loops, functions, classes, inheritance, static methods, constructors, this keyword, string operations, list operations, map operations, pattern matching, exception handling, closures, and more. All tests run automatically with CMake and CTest, and the CI pipeline runs them on every commit to catch regressions. The test results are displayed with clear pass/fail indicators and detailed error messages when something breaks.

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
| Builtins | 90 functions |
| Stdlib Modules | 21 |
| Examples | 38 programs |
| CLI Commands | 22 subcommands |
| Languages | 5 (en/am/es/fr/de) |
| Keywords | 26 per language |
| Opcodes | 42 |
| Token Types | 45 |
| Source Files | 33 (.cpp + .h) |
| LOC | 14,073 C++ |
| Optimization | Constant folding pass |

### CLI Commands

**v2.3.4 had only:** `alphabet --version`, `alphabet --repl`, `alphabet --lsp`, `alphabet --debug`, `alphabet --sandbox`, `alphabet --dump-bytecode`, `alphabet --compile`, `alphabet --output`, `alphabet update`

**v2.3.5 added 12 new subcommands:**

```
alphabet init <name>      Create new project (NEW)
alphabet run <file>       Run a program (NEW)
alphabet test [dir]       Run test files (NEW)
alphabet info             Show project info (NEW)
alphabet doc <name>       Show builtin docs (NEW)
alphabet bench            Run benchmarks (NEW)
alphabet examples         List examples (NEW)
alphabet tour             Interactive tour (NEW)
alphabet lint <file>      Lint for warnings (NEW)
alphabet pkg              Package manager (NEW)
alphabet voice-tutorial   Learn voice input (NEW)
alphabet setup-voice      Install voice dependencies (NEW)
```

**All 22 commands with examples:**

```bash
# Flags
alphabet --version        Show version
alphabet --repl           Interactive REPL
alphabet --lsp            LSP server for VS Code
alphabet --debug          Debug mode
alphabet --sandbox        Sandbox mode
alphabet --dump-bytecode  Inspect bytecode
alphabet --compile        Compile only
alphabet --output <file>  Output file

# Subcommands (NEW in v2.3.5)
alphabet init myproject           Create new project
alphabet run program.abc          Run a program
alphabet test tests/              Run test files
alphabet info                     Show project info
alphabet doc o                  Show builtin docs
alphabet bench                    Run benchmarks
alphabet examples                 List examples
alphabet tour                     Interactive tour
alphabet lint program.abc         Lint for warnings
alphabet pkg list                 Package manager
alphabet pkg search math          Search packages
alphabet pkg install math         Install packages
alphabet setup-voice              Install voice deps
alphabet voice-tutorial           Learn voice input
alphabet update                   Check for updates
alphabet update --force           Reinstall current version (fix bugs)
```

### REPL Commands

**v2.3.4 had only:** `q`, `quit`, `exit`, `help`, `clear`, `clear history`, `clear all`, `reset`, `vars`, `keywords`, `history`, `!!`

**v2.3.5 added 8 new commands:**

```
trace        Show trace mode status (NEW)
trace on     Enable execution trace (NEW)
trace off    Disable execution trace (NEW)
trace slow   Watch each step happen gradually (NEW)
trace fast   Instant results (default) (NEW)
lang         Show current language (NEW)
builtins     List all 82 builtins (NEW)
voice        Voice-to-text input (NEW)
reload       Clear variables, keep code (NEW)
```

**All 19 REPL commands:**

```bash
# v2.3.4 commands
q/quit/exit  Exit REPL
help         Show commands
clear        Clear screen
clear history Clear command history
clear all    Clear screen + history
reset        Clear everything
vars         Show defined variables
keywords     Show keywords for current language
history      Show command history
!!           Repeat last command

# v2.3.5 new commands
trace        Show trace mode status
trace on     Enable execution trace (on by default)
trace off    Disable execution trace
trace slow   Watch each step happen gradually
trace fast   Instant results (default)
lang         Show current language
builtins     List all 82 builtins
voice        Voice-to-text input (checks deps first)
reload       Clear variables, keep code
```

---

## v2.3.4 vs v2.3.5

This section compares what was available in v2.3.4 (May 28, 2026) and what was added or fixed in v2.3.5 (June 7, 2026).

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

v2.3.5 is a major release adding functional programming, closures, voice input, a browser playground, threading, and over 40 new features. Here is what each feature does and how to use it.

#### 1. super() - Call Parent Class Constructors

Inherit from a parent class and call its constructor using `super()`.

```alphabet
#alphabet<en>
c Base {
    v 5 value = 0
    m 1 Base(5 v) {
        this.value = v
    }
}
c Child ^ Base {
    m 1 Child(5 v) {
        super(v)
    }
    m 5 get_value() {
        r this.value
    }
}
5 c = n Child(42)
z.o(c.get_value())
```

Output: `42`

#### 2. Functional Programming - map, filter, reduce, closures

Transform lists with lambdas.

```alphabet
#alphabet<en>
m nums = [1, 2, 3, 4, 5]
m doubled = z.map(nums, m (x) { r x * 2 })
z.o(doubled)

m evens = z.filter(nums, m (x) { r x % 2 == 0 })
z.o(evens)

5 total = z.reduce(nums, 0, m (acc, x) { r acc + x })
z.o(total)
```

Output: `[2, 4, 6, 8, 10]` then `[2, 4]` then `15`

Lambdas capture global variables (closures):

```alphabet
#alphabet<en>
5 multiplier = 3
m multiply = m (x) { r x * multiplier }
z.o(multiply(5))
```

Output: `15`

#### 3. Ternary Operator

One-line conditional expressions.

```alphabet
#alphabet<en>
5 x = 10
5 result = x > 5 ? "big" : "small"
z.o(result)
```

Output: `big`

#### 4. Voice Input - Speak Code in 5 Languages

Speak code in English, Amharic, Spanish, French, or German.

```bash
alphabet setup-voice
alphabet --repl
>>> voice
# Speak: "print hello world"
```

Vosk for English/Spanish/French/German (offline). Whisper for Amharic (offline).

#### 5. Browser Playground - Run Code in Your Browser

```bash
open playground/index.html
```

Dark theme, 7 built-in examples, 5 language selector, Ctrl+Enter to run.

**Note:** The web IDE runs code through the native v2.3.5 binary via the server, giving you the FULL language experience in the browser.

#### 6. Threading Support - Concurrent Execution

```alphabet
#alphabet<en>
z.thread(m () { z.o("Hello from thread 1") })
z.thread(m () { z.o("Hello from thread 2") })
z.join_all()
```

Functions: `z.thread(fn)`, `z.join(id)`, `z.join_all()`, `z.lock(name)`, `z.acquire(name)`, `z.release(name)`

#### 7. Constant Folding - Compile-Time Optimization

The compiler evaluates constant expressions at compile time. `3 + 4` becomes `7` before running.

```alphabet
#alphabet<en>
5 x = (2 + 3) * 4
z.o(x)
```

Output: `20` - computed at compile time, no runtime cost.

#### 8. REPL Trace Mode - See How Compilation Works

The REPL shows you exactly what happens when you type code through 4 phases.

```
>>> 5 x = 10

┌─ Tokenizing ─────────────────────────────┐
│ 5 x = 10
└──────────────────────────────────────────┘
┌─ Parsing ────────────────────────────────┐
│ VarDecl(x: 5 = Lit(10))
└──────────────────────────────────────────┘
┌─ Compiling ──────────────────────────────┐
│ 0: PUSH_CONST 10
│ 1: STORE_VAR 0
│ 2: POP
│ 3: HALT
└──────────────────────────────────────────┘
┌─ Executing ──────────────────────────────┐
│ 0: PUSH_CONST 10  │ depth:0
│ 1: STORE_VAR 0    │ depth:1
│ 2: POP            │ depth:1
│ 3: HALT           │ depth:0
└──────────────────────────────────────────┘
```

**Commands:**
```
trace on      Enable trace (on by default)
trace off     Disable trace
trace slow    Watch each step happen gradually
trace fast    Instant results (default)
```

#### 9. Trace Slow Mode - Watch Compilation Happen

Use `trace slow` to see each token, AST node, and bytecode instruction appear one by one with delays.

```
>>> trace slow
>>> z.o("hello")

┌─ Tokenizing ─────────────────────────────┐
│ z . o ( hello )                           │  <- tokens appear one by one
└──────────────────────────────────────────┘  <- pause
┌─ Parsing ────────────────────────────────┐
│ ExprStmt(Call(Get(Var(z).o), [Lit("hello")])) │ <- appears
└──────────────────────────────────────────┘  <- pause
┌─ Compiling ──────────────────────────────┐
│ 0: PUSH_CONST "SYSTEM_Z"                 │  <- instructions appear one by one
│ 1: PUSH_CONST "hello"                    │
│ 2: PRINT                                 │
│ 3: POP                                   │
│ 4: HALT                                  │
└──────────────────────────────────────────┘  <- pause
┌─ Executing ──────────────────────────────┐
│ 0: PUSH_CONST "SYSTEM_Z"  │ depth:0      │  <- each step with stack depth
│ 1: PUSH_CONST "hello"     │ depth:1      │
│ 2: PRINT                   │ depth:2     │
│ → hello                                  │  <- output appears here
│ 3: POP                    │ depth:1      │
│ 4: HALT                   │ depth:0      │
└──────────────────────────────────────────┘
```

#### 10. Package Manager - Install Stdlib Modules

```bash
alphabet pkg list        # List available packages
alphabet pkg search math # Search for a package
alphabet pkg install math # Install a package
```

#### 11. Improved Error Messages - Keyword Conflict Hints

When you accidentally use a keyword as a variable name, the error tells you what went wrong.

```
>>> i = 10
Error at line 1, column 3: Expect expression.
  i = 10
    ^
  Hint: 'i' is the 'if' keyword. Use a different variable name.
```

Works for all 26 single-letter keywords.

#### 12. Embedding API - Use Alphabet in C++ Apps

```cpp
#include "alphabet_embed.h"
Alphabet VM;
VM.init();
VM.eval("z.o('Hello from C++')");
VM.run();
```

#### 13. Project Management

```bash
alphabet init myproject   # Create new project structure
alphabet info             # Show project info from alphabet.toml
```

#### 14. New Standard Library Modules

21 modules (up from 4 in v2.3.4) covering math, strings, JSON, collections, testing, and more.

```alphabet
#alphabet<en>
x math
5 r = math.sqrt(16)
z.o(r)

x json
m data = {"name": "Alice", "age": 30}
z.o(json.stringify(data))
```

Available: math, string_utils, json, collections, testing, validate, datetime, list_utils, math_ext, system, crypto, config, data_structures, functional, os, and more.

#### 15. Formal Language Specification

docs/SPEC.md - 1548 lines covering the complete language grammar, type system, and semantics.

#### 16. Bug Fixes

Critical bugs fixed in v2.3.5:

- **continue in for-loops**: Previously caused infinite loops
- **Static methods**: Previously returned null
- **Constructor dispatch**: Previously looked for `init` instead of class name
- **Nested for-each**: Variable name conflicts fixed with unique `__fe0`, `__fe1`
- **Abstract class parsing**: `a c ClassName` syntax now works
- **this.field type inference**: Compiler now correctly infers types

#### 17. REPL Improvements

- `voice` command checks dependencies before starting
- `builtins` command lists all 82 builtins by category
- `reload` command clears variables but keeps code
- `q` is now exclusively the `match` keyword (use `quit`/`exit` to leave)

#### 18. Error Recovery

Parser shows hints for common errors:

```
Missing closing parenthesis → "Hint: check for missing closing parenthesis"
Missing expression → "Hint: check for missing operator, value, or keyword conflict"
Missing variable name → "Hint: variable names can't be keywords"
Missing class name → "Hint: use 'n ClassName()' for constructors"
Missing braces → "Hint: check for missing opening/closing brace"
```

#### 19. LSP Server Improvements

- Startup message shows version and capabilities
- Supports: hover, completion, definition, symbols

#### 20. Windows Support

- `SetConsoleMode(ENABLE_VIRTUAL_TERMINAL_PROCESSING)` for ANSI colors

and match more fixes and improvement

### Summary Table

| Category | v2.3.4 | v2.3.5 | Added |
|----------|--------|--------|-------|
| CLI Subcommands | 1 | 13 | +12 |
| REPL Commands | 12 | 21 | +9 |
| Stdlib Modules | 4 | 21 | +17 |
| Test Files | 34 ctest | 34 ctest + 257 | +257 |
| LOC | ~14,000 | 14,073 | +73 |
| Examples | 0 | 38 | +38 |
| Playground | No | Yes | New |
| Voice Input | No | Yes | New |
| Trace Mode | No | Yes | New |
| Package Manager | No | Yes | New |
| Threading | No | Yes | New |
| Closures | No | Yes | New |
| Functional Prog | No | Yes | New |
| Embedding API | No | Yes | New |

---

## What Problem Does Alphabet Solve?

Alphabet addresses four fundamental problems that make programming harder than it needs to be. First, education overload: beginners struggle with syntax before understanding concepts, and Alphabet's 19-keyword design removes that barrier. Second, rapid prototyping: quick experiments without boilerplate code, letting you focus on ideas rather than ceremony. Third, multilingual programming: write code in your native language so you can learn logic without fighting English syntax. Fourth, tooling integration: built-in LSP server, debugger, and REPL work with any editor supporting the Language Server Protocol. These four problems affect millions of learners worldwide, and Alphabet solves them all with a single, minimal language that teaches concepts that transfer to every other programming language you will ever use.

### 1. Education Overload
Beginners struggle with syntax before understanding concepts. Alphabet removes this barrier with only 19 keywords.

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

Alphabet is an open source project that welcomes contributions from developers of all skill levels. Whether you want to fix a bug, add a feature, improve documentation, or report an issue, your help is appreciated. The project uses GitHub for collaboration —> you can report bugs through GitHub Issues, discuss ideas in GitHub Discussions, and submit code changes through pull requests. The CONTRIBUTING.md file provides detailed guidelines including project structure, coding standards, testing requirements, and the review process. First-time contributors are especially welcome, and good first issues are labeled to help you find accessible starting points.

- Report bugs: [GitHub Issues](https://github.com/fraol163/alphabet/issues)
- Discuss: [Discussions](https://github.com/fraol163/alphabet/discussions)

---

## License

Alphabet is released under the MIT License, one of the most permissive open source licenses available. This means you are free to use, modify, and distribute the language for any purpose —> personal, educational, or commercial  without restriction. You can embed Alphabet in your own projects, create derivative works, and even sell products that include it. The only requirement is that you include the original copyright notice and license text in any distribution. The full license text is available in the [LICENSE](LICENSE.txt) file in the project root.

---

## Contact

Alphabet is created and maintained by Fraol Teshome, a developer passionate about making programming education accessible to everyone regardless of their native language. You can reach out via email(fraolteshome444@gmail.com) for questions, feedback, collaboration ideas, or just to say hello. The project is hosted on GitHub where you can follow development, report issues, and join discussions. If you are using Alphabet in education, research, or a personal project, I would love to hear about your experience —> your feedback helps shape the future of the language.

---

**Built with C++17**

**Resources:** [Documentation](docs/) · [Examples](examples/) · [Source Code](src/) · [Standard Library](stdlib/)
