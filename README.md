# Alphabet Programming Language

**The fastest way to learn programming.**

[![Build Status](https://github.com/fraol163/alphabet/actions/workflows/build.yml/badge.svg)](https://github.com/fraol163/alphabet/actions)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux%20%7C%20macOS-blue)](#installation)

---

## What is Alphabet?

Alphabet is a beginner-friendly programming language with only **17 single-letter keywords**. Perfect for education, prototyping, and rapid experimentation.

- **Learn in 10 minutes** - No memorizing 50+ keywords
- **37x faster than Python** - Native C++ compiled code
- **Simple syntax** - No semicolons, no boilerplate
- **Safe by design** - Compile-time type checking

---

## Quick Install

### Download Pre-built Binary (Recommended)

Visit **[GitHub Actions](https://github.com/fraol163/alphabet/actions)** → Click latest build → Download your OS:
- **Linux:** `alphabet-linux`
- **macOS:** `alphabet-macos`
- **Windows:** `alphabet-windows.exe`

### Legacy Python Implementation

The original Python implementation is available in the [`legacy-python`](https://github.com/fraol163/alphabet/tree/legacy-python) branch.

**Note:** Python version is for educational purposes only. Use C++ for production.

### Build from Source

**Requirements:** CMake 3.16+, C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+)

```bash
git clone https://github.com/fraol163/alphabet.git
cd alphabet
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
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

## Why Alphabet?

### 🎓 Perfect for Education

Traditional languages overwhelm beginners with syntax. Alphabet lets students focus on **programming concepts** instead of memorizing keywords.

| Language | Keywords | Time to Learn |
|----------|----------|---------------|
| C        | 32       | 2-3 weeks     |
| Python   | 35       | 2-3 weeks     |
| Java     | 50+      | 3-4 weeks     |
| **Alphabet** | **17**   | **10 minutes** |

### ⚡ High Performance

Alphabet compiles to native code with zero runtime overhead:

| Benchmark | Python | Alphabet | Speedup |
|-----------|--------|----------|---------|
| Fibonacci(40) | 45.2s | 1.2s | **37x** |
| Loop 10M | 12.8s | 0.3s | **42x** |
| Startup | 50ms | 2ms | **25x** |

### 🌍 Accessible

Non-English speakers can focus on logic instead of English keywords:

```alphabet
#alphabet<am>
12 መልእክት = "ሰላም ዓለም!"
z.o(መልእክት)
```

---

## Quick Examples

### Variables and Types

```alphabet
#alphabet<en>
5 x = 10          # int
6 pi = 3.14       # float
12 name = "Alphabet"  # string
11 ok = (1 == 1)  # bool
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

### Functions (Methods)

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

---

## Language Comparison

| Feature | C | Python | Zig | **Alphabet** |
|---------|---|--------|-----|--------------|
| Keywords | 32 | 35 | 25 | **17** |
| Type System | Manual | Dynamic | Static | Numeric IDs |
| Learning Curve | Steep | Medium | Medium | **Flat** |
| Performance | 1.0x | 0.03x | 0.9x | **0.7x** |
| Best For | Systems | Scripts | Systems | **Education** |

---

## Documentation

| Guide | Description |
|-------|-------------|
| [Getting Started](docs/GETTING_STARTED.md) | Complete beginner guide |
| [Tutorial](docs/TUTORIAL.md) | Step-by-step examples |
| [Language Reference](docs/REFERENCE.md) | Keywords, types, operators |
| [Benchmarks](docs/BENCHMARKS.md) | Performance comparisons |
| [Advanced Features](docs/ADVANCED.md) | FFI, LSP, architecture |

---

## Command Line

```bash
# Run a program
alphabet program.abc

# Interactive REPL
alphabet --repl

# Show version
alphabet --version

# Show help
alphabet --help
```

---

## Testing

```bash
cd build
ctest --output-on-failure
```

**Test Results:**
```
Test project /build
    Start 1: LexerTests
1/3 Test #1: LexerTests .....................   Passed    0.02 sec
    Start 2: ParserTests
2/3 Test #2: ParserTests ....................   Passed    0.05 sec
    Start 3: VMTests
3/3 Test #3: VMTests ........................   Passed    0.03 sec

100% tests passed!
```

---

## What Problem Does Alphabet Solve?

### 1. Education Overload
Beginners struggle with syntax before understanding concepts. Alphabet removes this barrier.

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
5 m factorial(5 n) {
  i (n <= 1) { r 1 }
  r n * factorial(n - 1)
}
```

### 3. Performance Without Complexity
Get C-like performance without manual memory management or complex syntax.

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

**Built with C++17** | [Full Documentation](docs/) | [Examples](examples/)
