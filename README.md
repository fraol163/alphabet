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
# Alphabet Programming Language V2.3

**The fastest way to learn programming.**

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
  - [FFI - Call Native C Functions](#ffi---call-native-c-functions)
  - [Built-In Functions](#built-in-functions)
  - [Multilingual Keywords](#multilingual-keywords)
- [Standard Library](#standard-library)
- [Language Comparison](#language-comparison)
- [Command Line](#command-line)
- [Documentation](#documentation)
- [Testing](#testing)
- [What Problem Does Alphabet Solve?](#what-problem-does-alphabet-solve)
- [Contributing](#contributing)
- [License](#license)

---

## What is Alphabet?

Alphabet is a **beginner-friendly programming language** with only **17 single-letter keywords**. Designed for education, prototyping, and rapid experimentation, Alphabet makes programming accessible to everyone.

### Key Features

- **17 Keywords** - Master the entire language in 10 minutes
- **Bytecode VM** - Stack-based interpreter with compiled execution
- **Simple Syntax** - No semicolons, no boilerplate, no complex rules
- **Type System** - Compile-time type checking with numeric type IDs
- **Multilingual** - Write code in English, Amharic, Spanish, French, or German
- **LSP Support** - Works with VS Code and other editors
- **FFI** - Call native C functions from Alphabet code
- **Cross-Platform** - Works on Windows, Linux, and macOS
- **Built-in Debugger** - Breakpoints, step, locals, globals, stack trace
- **REPL** - Interactive mode with persistent state and error recovery
- **Standard Library** - Math, I/O, String, and List modules included
- **30+ Built-in Functions** - Range, split, join, replace, contains, keys, and more

### Why Choose Alphabet?

**For Students:** Focus on programming concepts instead of memorizing 50+ keywords like in traditional languages.

**For Educators:** Teach logic and algorithms without syntax overwhelming your students.

**For Prototypers:** Quick experiments without boilerplate. Test ideas in minutes, not hours.

**For Hobbyists:** Fun, lightweight language for personal projects and learning.

### Quick Comparison

| Task | Alphabet | Python | C |
|------|----------|--------|---|
| Keywords to learn | 17 | 35 | 32 |
| Time to hello world | 2 min | 5 min | 10 min |
| Best for | Education | Scripts | Systems |

---

## Installation

### Install (one command, Linux/macOS)

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

This installs to `~/.local/bin` (no sudo needed) and:
- Downloads the pre-built binary from GitHub Releases
- Falls back to building from source if no release exists
- Adds `~/.local/bin` to your PATH
- Shows version and usage instructions

### Update

Same command as install. It downloads and replaces with the latest version:

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

### Uninstall

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh -s -- --uninstall
```

### Manual Download

Visit **[GitHub Releases](https://github.com/fraol163/alphabet/releases)** → Download your OS:
- **Linux:** `alphabet-linux-amd64.tar.gz`
- **macOS:** `alphabet-macos-arm64.tar.gz`
- **Windows:** `alphabet-windows-amd64.zip`

### Build from Source

**Requirements:** CMake 3.16+, C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+)

```bash
git clone https://github.com/fraol163/alphabet.git
cd alphabet
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
sudo cmake --install build
alphabet --version
```

---

## Hello World

```alphabet
#alphabet<en>
12 greeting = "Hello, Alphabet!"
z.o(greeting)
```

**Output:**
```
Hello, Alphabet!
```

---

## Language Features

### Variables and Types

```alphabet
#alphabet<en>
5 x = 10              # int
6 pi = 3.14           # float
12 name = "Alphabet"  # string
11 ok = (1 == 1)      # bool
13 nums = [1, 2, 3]   # list
14 cfg = {"k": 100}   # map
```

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
# Output: 0, 2, 4, 6, 8
```

### For Loop

```alphabet
#alphabet<en>
l (5 j = 0 : j < 5 : j = j + 1) {
  z.o(j)
}
# Output: 0, 1, 2, 3, 4
```

### Break and Continue

```alphabet
#alphabet<en>
l (5 k = 0 : k < 10 : k = k + 1) {
  i (k == 3) { b }      # break at 3
  i (k % 2 == 0) { k }  # skip evens
  z.o(k)
}
```

### Functions

```alphabet
#alphabet<en>
m 5 factorial(5 num) {
  i (num <= 1) { r 1 }
  r num * factorial(num - 1)
}
z.o(factorial(5))  # Output: 120
```

### Classes

```alphabet
#alphabet<en>
c Calculator {
  v m 5 add(5 x, 5 y) {
    r x + y
  }
}

15 calc = n Calculator()
5 result = calc.add(15, 25)
z.o(result)  # Output: 40
```

### Pattern Matching

```alphabet
#alphabet<en>
5 x = 2
q (x) {
  1: z.o("one")
  2: z.o("two")
  e: z.o("other")
}
# Output: two
```

### Exception Handling

```alphabet
#alphabet<en>
t {
  z.o("in try")
} h (12 e) {
  z.o("caught: " + e)
}
```

### Import Modules

```alphabet
#alphabet<en>
x "path/to/module.abc"
5 result = imported_function(10)
```

### String Escapes and Concatenation

```alphabet
#alphabet<en>
z.o("Hello\nWorld")        # newline
z.o("x=" + 5)              # x=5
z.o(5 + " items")          # 5 items
```

### FFI - Call Native C Functions

```alphabet
#alphabet<en>
5 result = z.dyn("/path/to/lib.so", "add", 10, 20)
z.o(result)  # Output: 30
```

### Built-In Functions

```alphabet
#alphabet<en>
// Math
z.o(z.sqrt(144))    # 12
z.o(z.abs(-42))     # 42
z.o(z.pow(2, 10))   # 1024
z.o(z.floor(3.7))   # 3
z.o(z.ceil(3.2))    # 4

// Type info
z.o(z.len("hello")) # 5
z.o(z.type(42))     # "number"

// String operations
z.o(z.upper("hi"))          # "HI"
z.o(z.lower("HI"))          # "hi"
z.o(z.trim("  hi  "))       # "hi"
z.o(z.replace("a+b", "+", "-"))  # "a-b"
13 parts = z.split("a,b", ",")   # ["a", "b"]
z.o(z.join(parts, "-"))     # "a-b"
z.o(z.substr("hello", 1, 3))    # "ell"
z.o(z.chr(65))              # "A"
z.o(z.ord("A"))             # 65
z.o(z.starts_with("hello", "hel"))  # 1
z.o(z.ends_with("hello", "llo"))    # 1
z.o(z.contains("hello", "ell"))     # 1

// List operations
13 nums = [1, 2, 3]
z.append(nums, 4)           # [1,2,3,4]
z.o(z.pop_back(nums))       # 4
z.o(z.contains(nums, 2))    # 1
13 rev = z.reverse(nums)    # [3,2,1]

// Range
13 r = z.range(5)           # [0,1,2,3,4]
13 r2 = z.range(2, 7)       # [2,3,4,5,6]

// Map operations
14 m = {"a": 1, "b": 2}
13 k = z.keys(m)            # ["a", "b"]
13 v = z.values(m)          # [1, 2]
```

### Multilingual Keywords

Write code in your native language. Alphabet supports **5 languages** with full UTF-8 variable names.

#### How It Works

Change the header to your language:

```alphabet
#alphabet<am>    Amharic
#alphabet<es>    Spanish
#alphabet<fr>    French
#alphabet<de>    German
#alphabet<en>    English (default)
```

#### Keyword Mapping

| English | Amharic | Spanish | French | German |
|---------|---------|---------|--------|--------|
| `c` class | ክፍል | clase | classe | klasse |
| `m` method | ዘዴ | metodo | methode | methode |
| `i` if | ከሆነ | si | si | wenn |
| `e` else | አለበለዚህ | sino | sinon | sonst |
| `l` loop | ሉፕ | bucle | boucle | schleife |
| `r` return | ተመለስ | retornar | retour | zuruck |
| `b` break | ስበር | romper | rompre | brechen |
| `k` continue | ቀጥል | continuar | continuer | fortsetzen |
| `n` new | አዲስ | nuevo | nouveau | neu |
| `v` public | ግልጽ | publico | public | offentlich |
| `p` private | ግል | privado | prive | privat |
| `s` static | ቋሚ | estatico | statique | statisch |
| `t` try | ሞክር | intentar | essayer | versuchen |
| `h` handle | ይዝ | capturar | attraper | fangen |
| `z` output | ውጤት | imprimir | afficher | ausgeben |
| `x` import | አስገባ | importar | importer | importieren |
| `q` match | ምረጥ | coincidir | correspondre | ubereinstimmen |

#### Example: Amharic

```alphabet
#alphabet<am>
ክፍል አስላ {
  5 ውጤት = 0
  ዘዴ 5 መጀመሪያ(5 ቁ) {
    ውጤት = ቁ
    ተመለስ ቁ
  }
}
አስላ አ = አዲስ አስላ(42)
ውጤት.o(አ.ውጤት)  # Output: 42
```

#### UTF-8 Variable Names

Variable names can use any UTF-8 script:

```alphabet
#alphabet<en>
5 ቁጥር = 100        # Amharic variable name
5 数字 = 200         # Chinese variable name
5 число = 300       # Russian variable name
z.o(ቁጥር + 数字 + число)  # Output: 600
```

---

## Standard Library

Alphabet ships with a standard library in `stdlib/`:

### math.abc

```alphabet
#alphabet<en>
x "../stdlib/math.abc"
z.o(factorial(5))      # 120
z.o(gcd(48, 18))       # 6
z.o(lcm(4, 6))         # 12
z.o(max(10, 20))       # 20
z.o(clamp(15, 0, 10))  # 10
z.o(is_even(4))        # 1
z.o(sign(-5))          # -1
```

### io.abc

```alphabet
#alphabet<en>
x "../stdlib/io.abc"
print("hello")
12 content = read_file("data.txt")
```

### string.abc

```alphabet
#alphabet<en>
x "../stdlib/string.abc"
z.o(upper("hello"))              # "HELLO"
z.o(trim("  hi  "))              # "hi"
z.o(replace("a+b", "+", "-"))    # "a-b"
13 parts = split("a,b,c", ",")   # ["a","b","c"]
z.o(join(parts, "-"))            # "a-b-c"
z.o(starts_with("hello", "hel")) # 1
z.o(substr("hello", 1, 3))       # "ell"
```

### list.abc

```alphabet
#alphabet<en>
x "../stdlib/list.abc"
13 nums = range(5)          # [0,1,2,3,4]
push(nums, 99)              # [0,1,2,3,4,99]
z.o(pop(nums))              # 99
z.o(contains(nums, 2))      # 1
13 rev = reverse(nums)      # reversed copy
z.o(first(nums))            # 0
z.o(last(nums))             # 4
```

---

## Language Comparison

| Feature | C | Python | Zig | **Alphabet** |
|---------|---|--------|-----|--------------|
| Keywords | 32 | 35 | 25 | **17** |
| Type System | Manual | Dynamic | Static | **Numeric IDs** |
| Learning Curve | Steep | Medium | Medium | **Flat** |
| Best For | Systems | Scripts | Systems | **Education** |

---

## Command Line

```bash
# Run a program
alphabet program.abc

# Interactive REPL (state persists across lines)
alphabet --repl

# Start LSP server for editor integration
alphabet --lsp

# Debug with breakpoints
alphabet --debug program.abc

# Inspect compiled bytecode
alphabet --dump-bytecode program.abc

# Sandbox mode (block FFI and file access)
alphabet --sandbox program.abc

# Compile to binary bytecode
alphabet -c -o output.bin program.abc

# Self-update to latest version
alphabet update

# Show version
alphabet --version

# Show help
alphabet --help
```

### Environment Variables

| Variable | Description |
|----------|-------------|
| `ALPHABET_PATH` | Colon-separated directories to search for imports |

---

## Documentation

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

```bash
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

---

## What Problem Does Alphabet Solve?

### 1. Education Overload
Beginners struggle with syntax before understanding concepts. Alphabet removes this barrier with only 17 keywords.

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

We welcome contributions! See [CONTRIBUTING.md](CONTRIBUTING.md) for guidelines.

- Report bugs: [GitHub Issues](https://github.com/fraol163/alphabet/issues)
- Discuss: [Discussions](https://github.com/fraol163/alphabet/discussions)

---

## License

MIT License - See [LICENSE](LICENSE) file

---

## Contact

**Fraol Teshome**
Email: fraolteshome444@gmail.com
GitHub: [@fraol163](https://github.com/fraol163)

---

**Built with C++17**

**Resources:** [Documentation](docs/) · [Examples](examples/) · [Source Code](src/) · [Standard Library](stdlib/)
