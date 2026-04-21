# Alphabet Programming Language - Complete Documentation

**Version 2.3.0 | Native C++17 Implementation**

---

## Table of Contents

1. [What is Alphabet?](#1-what-is-alphabet)
2. [Getting Started](#2-getting-started)
3. [Language Fundamentals](#3-language-fundamentals)
4. [Data Types](#4-data-types)
5. [Variables](#5-variables)
6. [Operators](#6-operators)
7. [Control Flow](#7-control-flow)
8. [Functions](#8-functions)
9. [Classes and OOP](#9-classes-and-oop)
10. [Collections](#10-collections)
11. [String Operations](#11-string-operations)
12. [Exception Handling](#12-exception-handling)
13. [Pattern Matching](#13-pattern-matching)
14. [Module System](#14-module-system)
15. [Built-In Functions](#15-built-in-functions)
16. [Standard Library](#16-standard-library)
17. [Multilingual Keywords](#17-multilingual-keywords)
18. [FFI - Native C Integration](#18-ffi---native-c-integration)
19. [REPL Mode](#19-repl-mode)
20. [Debugging](#20-debugging)
21. [Command Line Interface](#21-command-line-interface)
22. [Architecture](#22-architecture)
23. [Type System Reference](#23-type-system-reference)
24. [Complete Keyword Reference](#24-complete-keyword-reference)

---

## 1. What is Alphabet?

Alphabet is a beginner-friendly programming language with only 17 single-letter keywords. It compiles to bytecode and runs on a stack-based virtual machine, written entirely in C++17.

### Design Philosophy
- **Simplicity first:** 17 keywords cover every concept you need
- **Learn by doing:** Write code in your native language (5 languages supported)
- **No boilerplate:** No semicolons, no imports for basic features, no build config
- **Real power:** Classes, exceptions, pattern matching, FFI, and a debugger

### Who Is It For?
- **Students** learning their first programming language
- **Educators** teaching computational thinking
- **Prototypers** who want quick experiments
- **Developers** building educational tools

---

## 2. Getting Started

### Installation

```bash
# One-line install (Linux/macOS)
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

This installs `alphabet` to `~/.local/bin` and adds it to your PATH.

### Your First Program

Create a file called `hello.abc`:

```alphabet
#alphabet<en>
12 greeting = "Hello, Alphabet!"
z.o(greeting)
```

Run it:

```bash
alphabet hello.abc
```

Output: `Hello, Alphabet!`

### The Magic Header

Every Alphabet file must start with `#alphabet<en>` on the first line. This tells the compiler which language keywords you are using.

```alphabet
#alphabet<en>     # English keywords
#alphabet<am>     # Amharic keywords
#alphabet<es>     # Spanish keywords
#alphabet<fr>     # French keywords
#alphabet<de>     # German keywords
```

---

## 3. Language Fundamentals

### Comments

```alphabet
// This is a comment (// anywhere on a line)
```

### Identifiers

Variable and function names can use:
- ASCII letters: `a-z`, `A-Z`
- Digits (not at start): `0-9`
- Underscore: `_`
- UTF-8 characters: `ቁጥር`, `数字`, `число`

### File Extension

All Alphabet source files use the `.abc` extension.

---

## 4. Data Types

Alphabet uses numeric type IDs to declare variables:

### Primitive Types

| Type ID | Name | Example |
|---------|------|---------|
| 5 | Integer | `5 x = 42` |
| 6 | Float (32-bit) | `6 pi = 3.14` |
| 7 | Float (64-bit) | `7 big = 1.23456789` |
| 11 | Boolean | `11 ok = 1` |
| 12 | String | `12 s = "hello"` |
| 13 | List | `13 nums = [1, 2, 3]` |
| 14 | Map | `14 cfg = {"key": 100}` |
| 15+ | Custom class | `15 obj = n MyClass()` |

### Type Inference

In practice, most users use:
- `5` for all numbers
- `12` for strings
- `13` for lists
- `14` for maps

---

## 5. Variables

### Declaration

```alphabet
#alphabet<en>
5 age = 25
12 name = "Fraol"
11 is_student = 1
```

### Reassignment

```alphabet
5 x = 10
5 x = x + 5     // x is now 15
```

### Constants

```alphabet
5 MAX = 100     // by convention, uppercase for constants
```

### Multiple Variables

```alphabet
5 a = 1
5 b = 2
5 c = 3
```

---

## 6. Operators

### Arithmetic

```alphabet
5 sum = 10 + 3      // 13
5 diff = 10 - 3     // 7
5 prod = 10 * 3     // 30
5 quot = 10 / 3     // 3
5 rem = 10 % 3      // 1
```

### Comparison

```alphabet
5 a = 5 == 5    // 1 (true)
5 b = 5 != 3    // 1 (true)
5 c = 5 > 3     // 1 (true)
5 d = 5 < 3     // 0 (false)
5 e = 5 >= 5    // 1 (true)
5 f = 5 <= 3    // 0 (false)
```

### Logical

```alphabet
5 x = 1 && 1    // 1 (AND)
5 y = 0 || 1    // 1 (OR)
5 z = !0        // 1 (NOT)
```

### String Concatenation

```alphabet
12 s = "Hello" + " " + "World"  // "Hello World"
12 t = "x = " + 42              // "x = 42"
```

---

## 7. Control Flow

### If/Else

```alphabet
5 x = 10
i (x > 5) {
  z.o("x is big")
} e {
  z.o("x is small")
}
```

### While Loop

```alphabet
5 i = 0
l (i < 5) {
  z.o(i)
  5 i = i + 1
}
```

### For Loop

```alphabet
l (5 j = 0 : j < 10 : j = j + 1) {
  z.o(j)
}
```

### Break and Continue

```alphabet
l (5 i = 0 : i < 100 : i = i + 1) {
  i (i == 3) { k }       // skip 3 (continue)
  i (i == 7) { b }       // stop at 7 (break)
  z.o(i)
}
```

### Nested Loops

```alphabet
l (5 i = 0 : i < 3 : i = i + 1) {
  l (5 j = 0 : j < 3 : j = j + 1) {
    z.o(i * 10 + j)
  }
}
```

---

## 8. Functions

### Defining Functions

```alphabet
m 5 add(5 a, 5 b) {
  r a + b
}
```

Syntax: `m <return_type> <name>(<param_type> <param>, ...) { body }`

### Calling Functions

```alphabet
5 result = add(3, 4)    // 7
z.o(result)
```

### Recursion

```alphabet
m 5 factorial(5 num) {
  i (num <= 1) { r 1 }
  r num * factorial(num - 1)
}
z.o(factorial(5))  // 120
```

### Functions as Values

Functions are first-class. You can pass them around:

```alphabet
m 5 double_it(5 x) { r x * 2 }
m 5 apply(5 val) { r double_it(val) }
z.o(apply(21))  // 42
```

---

## 9. Classes and OOP

### Basic Class

```alphabet
c Person {
  12 name = ""
  5 age = 0

  v m 12 greet() {
    r "Hi, I'm " + name
  }
}
```

### Creating Objects

```alphabet
15 p = n Person()
12 msg = p.greet()
```

### Visibility

```alphabet
c BankAccount {
  v 5 balance = 0       // public
  p 12 pin = "1234"     // private

  v m 5 deposit(5 amount) {
    5 balance = balance + amount
    r balance
  }
}
```

### Inheritance

```alphabet
c Animal {
  v m 12 speak() { r "..." }
}

c Dog ^ Animal {
  v m 12 speak() { r "Woof!" }
}
```

### Static Members

```alphabet
c Counter {
  s 5 count = 0

  v m 5 increment() {
    5 count = count + 1
    r count
  }
}
```

### Abstract Classes and Interfaces

```alphabet
a c Shape {
  m 5 area()
}

j Printable {
  m 12 to_string()
}
```

---

## 10. Collections

### Lists

```alphabet
// Create
13 nums = [1, 2, 3, 4, 5]

// Access by index
z.o(nums[0])      // 1
z.o(nums[-1])     // 5 (negative indexing!)

// Length
z.o(z.len(nums))  // 5

// Modify
z.append(nums, 6)      // [1,2,3,4,5,6]
z.o(z.pop_back(nums))  // 6

// Search
z.o(z.contains(nums, 3))  // 1

// Reverse
13 rev = z.reverse(nums)  // [5,4,3,2,1]
```

### Maps

```alphabet
// Create
14 config = {"host": "localhost", "port": 8080}

// Access
z.o(config["host"])  // "localhost"

// Modify
config["timeout"] = 30

// Keys and Values
13 k = z.keys(config)    // ["host", "port", "timeout"]
13 v = z.values(config)  // ["localhost", 8080, 30]
```

### Range

```alphabet
13 r = z.range(5)           // [0, 1, 2, 3, 4]
13 r2 = z.range(2, 8)       // [2, 3, 4, 5, 6, 7]
13 r3 = z.range(0, 10, 2)   // [0, 2, 4, 6, 8]
```

---

## 11. String Operations

```alphabet
12 s = "Hello, World!"

// Case conversion
z.o(z.upper(s))     // "HELLO, WORLD!"
z.o(z.lower(s))     // "hello, world!"

// Trimming
z.o(z.trim("  hi  "))  // "hi"

// Splitting and Joining
13 parts = z.split("a,b,c", ",")  // ["a", "b", "c"]
z.o(z.join(parts, "-"))            // "a-b-c"

// Replace
z.o(z.replace("cat and cat", "cat", "dog"))  // "dog and dog"

// Substring
z.o(z.substr("hello", 1, 3))  // "ell"

// Search
z.o(z.contains("hello", "ell"))      // 1
z.o(z.starts_with("hello", "hel"))   // 1
z.o(z.ends_with("hello", "llo"))     // 1

// Character conversion
z.o(z.chr(65))    // "A"
z.o(z.ord("A"))   // 65

// Reverse
z.o(z.reverse("abc"))  // "cba"
```

---

## 12. Exception Handling

```alphabet
t {
  5 x = 10 / 0
} h (12 e) {
  z.o("Error: " + e)
}
```

### Throwing Errors

```alphabet
m 5 divide(5 a, 5 b) {
  i (b == 0) { z.t("Division by zero") }
  r a / b
}
```

---

## 13. Pattern Matching

```alphabet
5 day = 3
q (day) {
  1: z.o("Monday")
  2: z.o("Tuesday")
  3: z.o("Wednesday")
  4: z.o("Thursday")
  5: z.o("Friday")
  e: z.o("Weekend")
}
```

The `e:` case is the default (else).

---

## 14. Module System

### Importing Files

```alphabet
#alphabet<en>
x "path/to/module.abc"
```

### Using Imported Functions

```alphabet
x "stdlib/math.abc"
z.o(factorial(5))    // function from math.abc
```

### Module Search Path

The compiler searches:
1. Relative to the source file
2. Directories in `ALPHABET_PATH` environment variable

---

## 15. Built-In Functions

All built-in functions are accessed through the `z` namespace.

### I/O
| Function | Description |
|----------|-------------|
| `z.o(x)` | Print value |
| `z.i()` | Read input |
| `z.f(path)` | Read file |

### Math
| Function | Description |
|----------|-------------|
| `z.sqrt(x)` | Square root |
| `z.abs(x)` | Absolute value |
| `z.pow(a, b)` | Power (a^b) |
| `z.floor(x)` | Floor |
| `z.ceil(x)` | Ceiling |
| `z.sin(x)` | Sine |
| `z.cos(x)` | Cosine |

### Type Info
| Function | Description |
|----------|-------------|
| `z.type(x)` | Type name as string |
| `z.len(x)` | Length of string/list/map |
| `z.tostr(x)` | Convert to string |
| `z.tonum(x)` | Convert to number |

### String
| Function | Description |
|----------|-------------|
| `z.upper(s)` | Uppercase |
| `z.lower(s)` | Lowercase |
| `z.trim(s)` | Trim whitespace |
| `z.split(s, d)` | Split string |
| `z.join(l, s)` | Join list |
| `z.replace(s, o, n)` | Replace all |
| `z.substr(s, i, l)` | Substring |
| `z.chr(n)` | Character from code |
| `z.ord(s)` | Code from character |
| `z.starts_with(s, p)` | Check prefix |
| `z.ends_with(s, s)` | Check suffix |
| `z.contains(c, v)` | Search (string or list) |
| `z.reverse(c)` | Reverse (string or list) |

### List/Map
| Function | Description |
|----------|-------------|
| `z.append(l, v)` | Add to list |
| `z.pop_back(l)` | Remove last |
| `z.range(stop)` | Range [0, stop) |
| `z.range(a, b)` | Range [a, b) |
| `z.range(a, b, s)` | Range with step |
| `z.keys(m)` | Map keys |
| `z.values(m)` | Map values |

### Other
| Function | Description |
|----------|-------------|
| `z.t()` | Throw error |
| `z.t(msg)` | Throw with message |
| `z.dyn(lib, func, ...)` | FFI call |

---

## 16. Standard Library

### math.abc
```alphabet
factorial(n)    # n!
gcd(a, b)       # Greatest common divisor
lcm(a, b)       # Least common multiple
max(a, b)       # Maximum
min(a, b)       # Minimum
clamp(v, lo, hi) # Clamp to range
is_even(n)      # 1 if even
is_odd(n)       # 1 if odd
sign(n)         # -1, 0, or 1
```

### io.abc
```alphabet
print(val)      # Print without newline
println(val)    # Print with newline
read_file(path) # Read file to string
```

### string.abc
```alphabet
contains(haystack, needle)
starts_with(s, prefix)
ends_with(s, suffix)
split(s, delim)
join(items, sep)
replace(s, old, new)
trim(s)
upper(s)
lower(s)
substr(s, start)
slice(s, start, length)
reverse(s)
length(s)
chr(code)
ord(c)
```

### list.abc
```alphabet
length(lst)
push(lst, val)
pop(lst)
contains(lst, val)
reverse(lst)
first(lst)
last(lst)
range(stop)
range_from(start, stop)
range_step(start, stop, step)
keys(map)
values(map)
```

---

## 17. Multilingual Keywords

Write code in your language. Each keyword maps to the same single-letter token.

| English | Amharic | Spanish | French | German |
|---------|---------|---------|--------|--------|
| class | ክፍል | clase | classe | klasse |
| if | ከሆነ | si | si | wenn |
| else | አለበለዚህ | sino | sinon | sonst |
| loop | ሉፕ | bucle | boucle | schleife |
| return | ተመለስ | retornar | retour | zuruck |
| new | አዲስ | nuevo | nouveau | neu |
| print | ውጤት | imprimir | afficher | ausgeben |

### UTF-8 Variable Names

```alphabet
#alphabet<en>
5 ቁጥር = 100
5 数字 = 200
z.o(ቁጥር + 数字)  # 300
```

---

## 18. FFI - Native C Integration

Call C functions directly from Alphabet:

```alphabet
#alphabet<en>
5 result = z.dyn("/usr/lib/libm.so", "sqrt", 144)
z.o(result)  # 12
```

Signature: `z.dyn(library_path, function_name, arg1, arg2, ...)`

Supports up to 4 int64_t arguments. Returns int64_t.

---

## 19. REPL Mode

```bash
alphabet --repl
```

Features:
- Persistent state across lines
- Multi-line input (brace tracking)
- Command history (saved to ~/.alphabet_history)
- `history` command to view past entries
- `!!` to repeat last command
- `q` to quit

---

## 20. Debugging

```bash
alphabet --debug program.abc
```

### Debugger Commands

| Command | Alias | Description |
|---------|-------|-------------|
| continue | c | Resume execution |
| step | s | Step to next line |
| locals | l | Show local variables |
| globals | g | Show global variables |
| stack | bt | Show call stack |
| print | p | Show stack contents |
| add_break N | b N | Set breakpoint at line N |
| del_break N | db N | Remove breakpoint |
| breakpoints | bl | List breakpoints |
| help | ? | Show help |

The debugger outputs JSON-structured events for tool integration.

---

## 21. Command Line Interface

```bash
alphabet [options] [file]

Options:
  -v, --version         Version info
  -h, --help            Help message
  -c, --compile         Compile only
  -o, --output FILE     Output file
  --repl                Interactive mode
  --lsp                 LSP server
  --debug               Debug mode
  --sandbox             Block FFI/file access
  --dump-bytecode       Print bytecode

Subcommands:
  alphabet update       Self-update
```

### Environment Variables

| Variable | Description |
|----------|-------------|
| `ALPHABET_PATH` | Colon-separated import directories |

---

## 22. Architecture

### Pipeline

```
Source (.abc) -> Lexer -> Tokens -> Parser -> AST -> Compiler -> Bytecode -> VM -> Output
```

### Components

- **Lexer** (476 lines): Tokenizes source with multilingual keyword support
- **Parser** (768 lines): Recursive descent parser producing AST
- **Compiler** (995 lines): AST to bytecode compiler with type checking
- **VM** (1235 lines): Stack-based interpreter with 43 opcodes
- **LSP** (509 lines): Language Server Protocol for editor integration
- **FFI** (278 lines): Dynamic library loading bridge

### Bytecode

43 opcodes including:
- Stack: PUSH_CONST, POP, DUP
- Arithmetic: ADD, SUB, MUL, DIV, PERCENT
- Comparison: EQ, NE, GT, GE, LT, LE
- Logic: AND, OR, NOT
- Control: JUMP, JUMP_IF_FALSE, JUMP_IF_TRUE, CALL, RET
- Objects: NEW, LOAD_FIELD, STORE_FIELD, GET_STATIC, SET_STATIC
- Collections: BUILD_LIST, BUILD_MAP, LOAD_INDEX, STORE_INDEX
- Exceptions: SETUP_TRY, POP_TRY, THROW
- Loops: LOOP_START, BREAK_JUMP, CONTINUE_JUMP

---

## 23. Type System Reference

| ID | Type | Runtime Storage |
|----|------|----------------|
| 0 | void | std::monostate |
| 1-4 | i8/i16/i32/i64 | double |
| 5 | int | double |
| 6-7 | f32/f64 | double |
| 8 | float | double |
| 9 | dec | double |
| 10 | cpx | double |
| 11 | bool | double (0/1) |
| 12 | str | std::string |
| 13 | list | shared_ptr<vector<Value>> |
| 14 | map | shared_ptr<unordered_map> |
| 15+ | class | ObjectPtr |

---

## 24. Complete Keyword Reference

| Token | Keyword (EN) | Description |
|-------|--------------|-------------|
| a | abstract | Abstract class/method |
| b | break | Exit loop |
| c | class | Define class |
| e | else | Else branch |
| h | handle/catch | Catch exception |
| i | if | Conditional |
| j | interface | Interface definition |
| k | continue | Skip to next iteration |
| l | loop/while | Loop |
| m | method/function | Define function |
| n | new | Create instance |
| p | private | Private access |
| r | return | Return value |
| s | static | Static member |
| t | try | Try block |
| v | public | Public access |
| z | output/print | System namespace |
| x | import | Import module |
| q | match | Pattern match |

---

**Alphabet Programming Language v2.3.0**
**By Fraol Teshome | MIT License | github.com/fraol163/alphabet**
