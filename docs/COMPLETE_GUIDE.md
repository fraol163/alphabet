# Alphabet Language - Complete Guide

**Comprehensive tutorial for Alphabet Programming Language v2.0**

---

## Table of Contents

1. [Introduction](#introduction)
2. [Installation](#installation)
3. [Getting Started](#getting-started)
4. [Basic Syntax](#basic-syntax)
5. [Data Types](#data-types)
6. [Control Flow](#control-flow)
7. [Functions](#functions)
8. [Classes & OOP](#classes--oop)
9. [Exception Handling](#exception-handling)
10. [Standard Library](#standard-library)
11. [Advanced Features](#advanced-features)
12. [Best Practices](#best-practices)

---

## Introduction

### What is Alphabet?

Alphabet is a **beginner-friendly programming language** with only **17 single-letter keywords**. It's designed for:

- 🎓 **Education** - Learn programming without syntax overload
- ⚡ **Prototyping** - Quick experiments and testing ideas
- 🚀 **Performance** - Native C++ compiled code
- 🌐 **Accessibility** - Easy for non-English speakers

### Why 17 Keywords?

Traditional languages require memorizing 30-50 keywords:
- C: 32 keywords
- Python: 35 keywords
- Java: 50+ keywords

Alphabet uses only **17 single letters**, making it possible to learn the entire language in **10 minutes**.

### Design Philosophy

1. **Simplicity First** - Easy to learn, easy to use
2. **Performance Matters** - Native compilation, not interpreted
3. **Type Safety** - Catch errors at compile time
4. **Minimal Boilerplate** - No semicolons, no complex syntax

---

## Installation

### Quick Install (Recommended)

Download pre-built binary from [GitHub Actions](https://github.com/fraol163/alphabet/actions):

1. Click latest build
2. Download for your OS
3. Run `alphabet --version`

### Build from Source

```bash
# Install dependencies
sudo apt install cmake g++ make  # Linux
brew install cmake               # macOS

# Clone and build
git clone https://github.com/fraol163/alphabet.git
cd alphabet/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

### Verify Installation

```bash
alphabet --version
```

**Expected output:**
```
Alphabet 2.1.0 (Native C++)
Developer: Fraol Teshome (fraolteshome444@gmail.com)
```

---

## Getting Started

### Your First Program

Create `hello.abc`:

```alphabet
#alphabet<en>
12 greeting = "Hello, World!"
z.o(greeting)
```

Run it:

```bash
alphabet hello.abc
```

**Output:**
```
Hello, World!
```

### File Structure

- **Extension:** `.abc`
- **Magic Header:** `#alphabet<en>` (required on line 1)
- **Encoding:** UTF-8

---

## Basic Syntax

### Variables

```alphabet
#alphabet<en>
5 x = 10          # Integer
6 pi = 3.14       # Float
12 name = "Alphabet"  # String
11 ok = (1 == 1)  # Boolean
```

### Output

```alphabet
z.o("Hello")      # Print string
z.o(x)            # Print variable
z.o(x + y)        # Print expression
```

### Input

```alphabet
5 x = z.i()       # Read integer
12 s = z.i()      # Read string
```

---

## Data Types

### Numeric Type IDs

Alphabet identifies types by numeric ID:

| ID | Type | Size | Example |
|----|------|------|---------|
| 1 | i8 | 8-bit | `1 x = 100` |
| 2 | i16 | 16-bit | `2 x = 10000` |
| 3 | i32 | 32-bit | `3 x = 50000` |
| 4 | i64 | 64-bit | `4 x = 1000000` |
| 5 | int | Generic | `5 x = 42` |
| 6 | f32 | 32-bit float | `6 pi = 3.14` |
| 7 | f64 | 64-bit float | `7 pi = 3.14159` |
| 8 | float | Generic float | `8 x = 0.5` |
| 11 | bool | Boolean | `11 ok = (1 == 1)` |
| 12 | str | String | `12 s = "Hello"` |
| 13 | list | Array | `13 arr = [1, 2, 3]` |
| 14 | map | Dictionary | `14 m = {"id": 1}` |
| 15+ | custom | Class | `15 obj = n MyClass()` |

---

## Control Flow

### If Statements

```alphabet
i (x > 0) {
  z.o("positive")
} e {
  z.o("non-positive")
}
```

### Loops

```alphabet
5 i = 0
l (i < 10) {
  z.o(i)
  5 i = i + 1
}
```

### Break and Continue

```alphabet
l (1 == 1) {
  i (x > 100) {
    b  # Break out of loop
  }
  i (x % 2 == 0) {
    k  # Skip to next iteration
  }
  5 x = x + 1
}
```

---

## Functions

### Define Methods in Classes

```alphabet
c Calculator {
  v m 5 add(5 a, 5 b) {
    r a + b
  }
  
  v m 5 subtract(5 a, 5 b) {
    r a - b
  }
}

15 calc = n Calculator()
5 result = calc.add(15, 25)
z.o(result)  # Output: 40
```

### Access Modifiers

- `v` (Public) - Universal access
- `p` (Private) - Restricted to class

---

## Classes & OOP

### Define a Class

```alphabet
c Person {
  p 12 name = ""
  p 5 age = 0
  
  v m 0 init(12 n, 5 a) {
    p 12 name = n
    p 5 age = a
  }
  
  v m 12 getName() {
    r name
  }
  
  v m 0 introduce() {
    z.o("Hi, I'm " + name)
  }
}
```

### Create Objects

```alphabet
15 alice = n Person()
alice.init("Alice", 25)
alice.introduce()  # Output: Hi, I'm Alice
```

---

## Exception Handling

### Try-Handle

```alphabet
t {
  # Risky code
  z.t()  # Throw error
} h (12 e) {
  z.o("Caught error: " + e)
}
```

---

## Standard Library

### System Functions (z)

| Function | Description | Example |
|----------|-------------|---------|
| `z.o(x)` | Output | `z.o("Hello")` |
| `z.i()` | Input | `5 x = z.i()` |
| `z.f(p)` | Read file | `12 data = z.f("file.txt")` |
| `z.t()` | Throw error | `z.t()` |

---

## Advanced Features

### Foreign Function Interface (FFI)

```alphabet
15 libc = z.load_lib("libc.so.6")
libc.call("printf", "Hello from C!\n")
```

### Language Server Protocol (LSP)

```bash
alphabet --lsp
```

Enables real-time error highlighting in VS Code.

---

## Best Practices

### Code Style

✅ **Do:**
- Start files with `#alphabet<en>`
- Use ASCII for identifiers
- Use meaningful variable names
- Keep functions short
- Comment complex logic

❌ **Don't:**
- Use Unicode identifiers (not supported)
- Forget the magic header
- Use semicolons (not needed)
- Deep recursion (>1000 levels)

### Performance Tips

1. Use smaller type IDs (faster)
2. Minimize object creation in loops
3. Use local variables
4. Prefer iteration over recursion

---

## Next Steps

- **Practice:** See [examples/](../examples/)
- **Reference:** [REFERENCE.md](REFERENCE.md)
- **Benchmarks:** [BENCHMARKS.md](BENCHMARKS.md)
- **Advanced:** [ADVANCED.md](ADVANCED.md)

---

**Happy Coding! 🚀**
