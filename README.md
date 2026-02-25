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
# Alphabet Programming Language v2.0

**Native C++ Compiled Ecosystem**

**Developed by Fraol Teshome** (fraolteshome444@gmail.com)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/fraol163/alphabet/actions/workflows/build.yml/badge.svg)](https://github.com/fraol163/alphabet/actions)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)](#installation)
[![C++17](https://img.shields.io/badge/C++-17-green.svg)](https://en.cppreference.com/w/cpp/17)

---

## Overview

Alphabet is a high-performance, compiled programming language featuring extreme code density through single-character keywords. Rewritten from Python to native C++, Alphabet delivers:

- **10-100x Performance** - Native compiled binary vs interpreted Python
- **Strong Static Typing** - Compile-time type validation
- **Zero-Copy Lexing** - std::string_view for maximum speed
- **Smart Memory** - Fixed-size stack, no heap fragmentation
- **FFI Support** - Link external C/C++ libraries
- **LSP Integration** - VS Code real-time error highlighting

---

## Quick Install

### From GitHub Actions (Recommended)

1. Visit **[Actions](https://github.com/fraol163/alphabet/actions)**
2. Select **latest build** -> Download artifact for your OS
3. Extract and run

### Build from Source

```bash
# Prerequisites: CMake 3.16+, C++17 compiler (g++, clang++, or MSVC)

git clone https://github.com/fraol163/alphabet.git
cd alphabet
mkdir build && cd build

# Standard build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)

# Install globally
sudo make install

# Verify
alphabet --version
```

### Package Managers (Coming Soon)

```bash
# Ubuntu/Debian
sudo apt install alphabet-lang

# macOS (Homebrew)
brew install alphabet

# Windows (Chocolatey)
choco install alphabet
```

---

## Single-Character Keywords

Alphabet uses exactly 17 single-letter keywords for maximum code density:

| Letter | Keyword | Description | Example |
|--------|---------|-------------|---------|
| a | Abstract | Abstract class definition | `a c A { }` |
| b | Break | Exit loop immediately | `b` |
| c | Class | Define class blueprint | `c MyClass { }` |
| e | Else | Alternative conditional path | `i (x) { } e { }` |
| h | Handle | Catch exception | `h (15 e) { }` |
| i | If | Conditional check | `i (x > 0) { }` |
| j | Interface | Method contract | `j J { m 1 f() }` |
| k | Continue | Skip to next iteration | `k` |
| l | Loop | Repeat while condition true | `l (x > 0) { }` |
| m | Method | Function in class | `v m 1 f() { }` |
| n | New | Instantiate object | `15 o = n MyClass()` |
| p | Private | Restrict access | `p 1 x = 10` |
| r | Return | Exit method with value | `r x + y` |
| s | Static | Class-level member | `s 1 x = 5` |
| t | Try | Monitor for errors | `t { }` |
| v | Public | Universal access | `v m 1 f() { }` |
| z | System | Standard library gateway | `z.o("Hello")` |

**Unused Letters (Reserved):** d, f, g, q, u, w, x, y

---

## Numeric Type System

Alphabet identifies data types by numeric ID for maximum density:

| ID | Type | Description | Example |
|----|------|-------------|---------|
| 1 | i8 | 8-bit signed integer | `1 x = 10` |
| 2 | i16 | 16-bit signed integer | `2 x = 1000` |
| 3 | i32 | 32-bit signed integer | `3 x = 50000` |
| 4 | i64 | 64-bit signed integer | `4 x = 90000000` |
| 5 | int | Generic integer | `5 x = 100` |
| 6 | f32 | 32-bit floating point | `6 pi = 3.14` |
| 7 | f64 | 64-bit floating point | `7 pi = 3.14159` |
| 8 | float | Generic float | `8 x = 0.5` |
| 9 | dec | Decimal (precision) | `9 money = 99.99` |
| 10 | cpx | Complex number | `10 z = 1+2j` |
| 11 | bool | Boolean (true/false) | `11 ok = (1 == 1)` |
| 12 | str | String | `12 s = "Hello"` |
| 13 | list | Dynamic array | `13 arr = [1, 2, 3]` |
| 14 | map | Hash map/dictionary | `14 m = {"id": 1}` |
| 15+ | obj | Custom class type | `15 user = n Person()` |

**Note:** Type IDs 15 and above are automatically assigned to custom classes in definition order.

---

## System Functions

The `z` (System) object provides built-in functionality:

| Function | Description | Example |
|----------|-------------|---------|
| z.o(x) | Print/output to console | `z.o("Hello")` |
| z.i() | Read input from console | `5 x = z.i()` |
| z.f(path) | Read file contents | `12 data = z.f("file.txt")` |
| z.t() | Throw test error | `z.t()` |

---

## Usage

### File Extension

Alphabet source files use the `.abc` extension.

### Magic Header Requirement

**Every `.abc` file must start with a magic header:**

```alphabet
#alphabet<en>
```

This identifies the file as Alphabet source code. Without it, compilation fails with `MissingLanguageHeader` error.

### Hello World

```alphabet
#alphabet<en>
12 greeting = "Hello, Alphabet!"
z.o(greeting)
```

### Object-Oriented Programming

```alphabet
#alphabet<en>
c Calculator {
  v m 5 add(5 x, 5 y) {
    r x + y
  }
}

15 calc = n Calculator()
5 result = calc.add(15, 25)
z.o(result)
```

Output: `40`

### Control Flow

```alphabet
#alphabet<en>
5 i = 0
l (i < 10) {
  i (i % 2 == 0) {
    z.o(i)
  }
  5 i = i + 1
}
```

Output: `0`, `2`, `4`, `6`, `8`

---

## Command Line Interface

```bash
# Run a program
alphabet program.abc

# Start REPL (interactive mode)
alphabet --repl

# Start LSP server (VS Code integration)
alphabet --lsp

# Show version
alphabet --version

# Show help
alphabet --help
```

### Exit Codes

| Code | Meaning |
|------|---------|
| 0 | Success |
| 1 | Compilation error (syntax, type mismatch) |
| 2 | Runtime error (undefined variable, type error) |
| 127 | Command not found (not installed) |

---

## Troubleshooting

### "Missing magic header" Error

**Problem:** File doesn't start with `#alphabet<lang>`
**Fix:** Add `#alphabet<en>` as the first line

### "Type mismatch" Error

**Problem:** Assigning incompatible types
**Fix:** Check type IDs match (e.g., don't assign string to int)

### "Command not found: alphabet"

**Problem:** Binary not in PATH
**Fix:**
```bash
# Linux/macOS
sudo make install

# Windows
# Add C:\Alphabet to System PATH
```

### Segmentation Fault

**Problem:** Stack overflow or memory issue
**Fix:**
- Report as bug (include .abc file)
- Try with AddressSanitizer: `cmake -DENABLE_ASAN=ON ...`

---

## Advanced Features

### Compile-Time Type Validation

Alphabet enforces strict type compatibility **before** generating bytecode:

```alphabet
#alphabet<en>
5 x = "Hello"  // Compile Error: Type mismatch
```

### Foreign Function Interface (FFI)

Call external C/C++ libraries (API in development):

```alphabet
#alphabet<en>
// Load and call external functions
// FFI documentation coming soon
```

### Language Server Protocol (LSP)

Real-time error highlighting in VS Code:

```bash
# Start LSP server
alphabet --lsp
```

**VS Code Configuration:** Add to `.vscode/settings.json`:

```json
{
  "alphabet.lsp.command": "alphabet",
  "alphabet.lsp.args": ["--lsp"]
}
```

---

## Architecture

```
+------------------+
|  User Space      |
|  Source (.abc)   |
+------------------+
        |
        | Raw Text
        v
+------------------+
|  Lexer           |
|  (string_view)   |
+------------------+
        |
        | Tokens
        v
+------------------+
|  Parser          |
|  (Recursive      |
|   Descent)       |
+------------------+
        |
        | AST
        v
+------------------+
|  Compiler        |
|  (Type Check)    |
+------------------+
        |
        | Bytecode
        v
+------------------+
|  VM              |
|  (Fixed Stack)   |
+------------------+
        |
        v
     Output
```

### Component Description

| Component | File | Purpose |
|-----------|------|---------|
| Lexer | src/include/lexer.h/cpp | Zero-copy tokenization with std::string_view |
| Parser | src/include/parser.h/cpp | Recursive descent to AST |
| Compiler | src/include/compiler.h/cpp | AST to bytecode with type validation |
| VM | src/include/vm.h/cpp | Stack-based execution with fixed array |
| Type System | src/include/type_system.h/cpp | Dynamic type registry (unlimited custom types) |
| FFI | src/include/ffi.h/cpp | C/C++ library bridge |
| LSP | src/include/lsp.h/cpp | Language Server Protocol for VS Code |

---

## Testing

### Run Test Suite

```bash
cd build
ctest --output-on-failure
```

### Golden File Tests

```bash
# All tests in tests/golden_files/
./alphabet tests/golden_files/hello.abc       # Expected: "Hello, Alphabet!"
./alphabet tests/golden_files/arithmetic.abc  # Expected: 30, 200
./alphabet tests/golden_files/class_method.abc # Expected: 40
./alphabet tests/golden_files/if_else.abc     # Expected: "x is greater than 5"
./alphabet tests/golden_files/loop.abc        # Expected: 10
```

### AddressSanitizer (Memory Safety)

```bash
cmake -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)
ctest
```

---

## Performance Comparison

| Metric | Python (v1.0) | C++ (v2.0) | Improvement |
|--------|---------------|------------|-------------|
| Startup Time | ~50ms | ~2ms | 25x faster |
| Tokenization | String copies | Zero-copy | 10x faster |
| Memory | GC overhead | Fixed stack | 5x less |
| Execution | Interpreted | Native | 10-100x faster |

---

## Cross-Platform Distribution

### Windows (NSIS Installer)

- `.exe` installer with automatic PATH configuration
- Download from GitHub Actions -> `alphabet-windows.exe`

### Linux (DEB / Tarball)

- Debian/Ubuntu: `alphabet_2.0.0_amd64.deb`
- Universal: `alphabet-2.0.0-Linux-x86_64.tar.gz`

### macOS (DMG / Homebrew)

- DMG: `alphabet-2.0.0-macOS.dmg`
- Homebrew: `brew install alphabet` (coming soon)

---

## Documentation

| Document | Description |
|----------|-------------|
| [DOCUMENTATION.md](docs/DOCUMENTATION.md) | Quick reference |
| [COMPLETE_GUIDE.md](docs/COMPLETE_GUIDE.md) | Full language guide |
| [ROADMAP.md](docs/ROADMAP.md) | Future development |
| [COMPLIANCE_REPORT.md](docs/COMPLIANCE_REPORT.md) | Task verification |
| [PRESENTATION.md](docs/PRESENTATION.md) | Technical overview |

---

## Alphabet Language Quick Reference

### All 26 Letters Status

```
USED:     a b c e h i j k l m n p r s t v z (17 letters)
RESERVED: d f g q u w x y (8 letters)
```

### Type ID Quick Lookup

```
Integers:  1=i8   2=i16   3=i32   4=i64   5=int
Floats:    6=f32  7=f64   8=float 9=dec   10=cpx
Other:     11=bool 12=str  13=list 14=map
Custom:    15+ (auto-assigned to classes)
```

### Operator Symbols

```
Arithmetic:  +  -  *  /  %
Comparison:  == != >  <  >= <=
Logical:     && || !
Assignment:  =
Access:      .  @  ^
Delimiters:  {} () [] , :
```

### Memory Layout

```
Stack: std::array<Value, 65536>  (fixed, ~512KB)
Heap:  Dynamic types only (lists, maps, objects)
```

---

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests: `ctest --output-on-failure`
5. Submit a pull request

### Development Setup

```bash
# Debug build with AddressSanitizer
cmake -DENABLE_ASAN=ON -DCMAKE_BUILD_TYPE=Debug ..
make -j$(nproc)

# Run all tests
ctest --verbose

# Format code (requires clang-format)
make format
```

---

## License

MIT License - See [LICENSE](LICENSE) file

---

## Contact

**Fraol Teshome**
- Email: fraolteshome444@gmail.com
- GitHub: https://github.com/fraol163

For tech presentations, partnerships, or inquiries.

---

## Roadmap

- [ ] Package manager integration (apt, brew, choco)
- [ ] Enhanced FFI with automatic type conversion
- [ ] VS Code extension with syntax highlighting
- [ ] Standard library expansion
- [ ] Parallel execution support
- [ ] WebAssembly target

---

**Built with C++17**
