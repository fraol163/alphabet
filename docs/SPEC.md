# Alphabet Programming Language — Formal Specification

> Version 2.3.5 · This document is the authoritative language reference.
> Compiler pipeline: Source → Lexer → Parser → AST → Compiler → Bytecode → VM

---

## Table of Contents

1. [Overview](#1-overview)
2. [Lexical Structure](#2-lexical-structure)
3. [Type System](#3-type-system)
4. [Variables and Constants](#4-variables-and-constants)
5. [Expressions](#5-expressions)
6. [Control Flow](#6-control-flow)
7. [Functions](#7-functions)
8. [Classes](#8-classes)
9. [Closures and Lambdas](#9-closures-and-lambdas)
10. [Built-in Functions](#10-built-in-functions)
11. [Error Handling](#11-error-handling)
12. [Import System](#12-import-system)
13. [Multilingual Keywords](#13-multilingual-keywords)
14. [Bytecode VM Architecture](#14-bytecode-vm-architecture)
15. [Grammar (EBNF)](#15-grammar-ebnf)

---

## 1. Overview

Alphabet is a **multilingual, education-focused programming language** that lets
programmers write code in their native language — English, Amharic, Spanish,
French, or German — while learning universal programming concepts. It compiles
to bytecode and executes on a stack-based virtual machine implemented in C++17.

### Key Design Principles

- **Multilingual Keywords:** 19 core keywords each mapped to 5 human languages.
- **Numeric Type IDs:** Every type has a numeric identifier (e.g. `5` = integer,
  `12` = string). Named aliases (`int`, `str`, `bool`, `list`, `map`, `float`)
  are also supported.
- **Single-Letter Keywords:** The canonical internal form uses single-letter
  keywords (`c`, `m`, `i`, `e`, `l`, `r`, `b`, `k`, `n`, `v`, `p`, `s`, `t`,
  `h`, `z`, `x`, `q`, `a`, `j`), making programs compact.
- **UTF-8 Identifiers:** Variable and function names may use any Unicode script.
- **Stack-Based Bytecode VM:** Programs are compiled to a flat bytecode format
  and executed on a register-free virtual machine with a 64K-entry stack.

### Source File Convention

Alphabet source files use the `.abc` extension. Every file must begin with a
language header on line 1:

```
#alphabet<LANG>
```

where `LANG` is one of `en`, `am`, `es`, `fr`, or `de`. The header tells the
lexer which keyword mapping to use for translation.

### Example — Hello World

```alphabet
#alphabet<en>
12 greeting = "Hello, Alphabet!"
o(greeting)
```

---

## 2. Lexical Structure

### 2.1 Character Set

Alphabet source files are UTF-8 encoded. Identifiers may contain any Unicode
letter or digit. Keywords are ASCII and are translated from the human-language
form to their canonical single-letter form by the lexer.

### 2.2 Comments

```
single_line   ::= "//" <any characters until end of line>
multi_line    ::= "/*" <any characters until "*/"> "*/"
```

Comments are ignored by the lexer.

### 2.3 Literals

| Literal     | Syntax                                      | Example        |
|-------------|----------------------------------------------|----------------|
| Integer     | `DIGIT+` (optionally separated by `_`)      | `42`, `1_000`  |
| Float       | `DIGIT+ . DIGIT+ [eE [+|-] DIGIT+]?`       | `3.14`, `1e10` |
| String      | `" ... "` or `' ... '` with escape sequences | `"hello\n"`    |
| Raw String  | `r" ... "` — no escape processing            | `r"C:\path"`   |
| F-String     | `f" ... {expr} ... "` — interpolation        | `f"x={x}"`     |
| Boolean     | `true` / `false`                             | `true`         |
| Null        | `null`                                       | `null`         |
| List        | `[ expr, expr, ... ]`                        | `[1, 2, 3]`    |
| Map         | `{ key: value, ... }`                        | `{"a": 1}`     |
| Range       | `expr .. expr`                               | `1..10`        |

#### Escape Sequences (in quoted strings)

| Sequence | Meaning         |
|----------|-----------------|
| `\n`     | newline         |
| `\r`     | carriage return |
| `\t`     | tab             |
| `\b`     | backspace       |
| `\\`     | literal `\`     |
| `\"`     | literal `"`     |
| `\'`     | literal `'`     |

### 2.4 Operators

| Operator | Description          | Operator | Description          |
|----------|----------------------|----------|----------------------|
| `+`      | Add / Concatenate    | `==`     | Equal                |
| `-`      | Subtract / Negate    | `!=`     | Not equal            |
| `*`      | Multiply             | `<`      | Less than            |
| `/`      | Divide               | `>`      | Greater than         |
| `%`      | Modulo               | `<=`     | Less or equal        |
| `=`      | Assignment           | `>=`     | Greater or equal     |
| `.`      | Field access         | `&&`     | Logical AND          |
| `?.`     | Null-safe access     | `\|\|`   | Logical OR           |
| `..`     | Range                | `!`      | Logical NOT          |
| `^`      | Inheritance (`extends`) | `@`   | Export               |
| `?`      | Ternary / optional   | `:`      | Map entry / match    |

### 2.5 Delimiters

`{` `}` `(` `)` `[` `]` `,` `;` (reserved, not commonly used)

### 2.6 Identifiers

```
IDENT  ::= ( LETTER | "_" ) ( LETTER | DIGIT | "_" )*
LETTER ::= [a-zA-Z] | any Unicode letter
DIGIT  ::= [0-9]
```

Identifiers are case-sensitive. UTF-8 identifiers are fully supported.

---

## 3. Type System

Alphabet uses a **numeric type ID system** where every type is identified by an
integer. Named type keywords (`int`, `str`, `bool`, etc.) are sugar for the
numeric IDs. Custom (class) types start at ID 15.

### 3.1 Built-in Types

| ID   | Name      | Description                     |
|------|-----------|---------------------------------|
| 0    | `void`    | No return value                 |
| 1    | `i8`      | 8-bit signed integer            |
| 2    | `i16`     | 16-bit signed integer           |
| 3    | `i32`     | 32-bit signed integer           |
| 4    | `i64`     | 64-bit signed integer           |
| 5    | `int`     | Integer (default, alias for i64)|
| 6    | `f32`     | 32-bit float                    |
| 7    | `f64`     | 64-bit float                    |
| 8    | `float`   | Float (default, alias for f64)  |
| 9    | `dec`     | Decimal                         |
| 10   | `cpx`     | Complex number                  |
| 11   | `bool`    | Boolean (`true` / `false`)      |
| 12   | `str`     | String (UTF-8)                  |
| 13   | `list`    | Ordered collection              |
| 14   | `map`     | Key-value dictionary            |
| 15+  | (custom)  | User-defined class types        |

### 3.2 Type Compatibility

- `i8`..`i64`, `f32`..`f64`, `bool` are all **numeric** and mutually coercible.
- `list` and `map` are generic containers holding `Value` elements.
- Custom class types inherit from a root class and may implement interfaces.
- A value of any numeric type may be used where a numeric type is expected;
  the VM performs implicit widening.

### 3.3 Named Type Aliases

The following named keywords are accepted in place of numeric type IDs:

| Keyword  | Numeric ID | Also Known As |
|----------|------------|---------------|
| `void`   | 0          | —             |
| `int`    | 5          | `integer`     |
| `float`  | 8          | —             |
| `bool`   | 11         | `boolean`     |
| `str`    | 12         | `string`      |
| `list`   | 13         | —             |
| `map`    | 14         | —             |

---

## 4. Variables and Constants

### 4.1 Variable Declaration

Variables are declared with a type keyword (numeric or named) followed by an
identifier and an initializer:

```
TYPE ID NAME = EXPRESSION
```

```alphabet
5 x = 10
12 name = "Alice"
13 items = [1, 2, 3]
14 config = {"key": "value"}
```

The type keyword may also be a named alias:

```alphabet
int x = 10
str name = "Alice"
list items = [1, 2, 3]
```

### 4.2 Reassignment

Variables declared with `TYPE ID NAME = ...` are **mutable**. Reassign with `=`:

```alphabet
5 x = 10
x = 20
```

### 4.3 Constants

The `const` keyword (internal: `\x80`) creates an immutable binding. The
compiler enforces that constants are never reassigned:

```alphabet
const 5 MAX_SIZE = 100
const 12 GREETING = "hello"
```

Constants may also be written with named type keywords:

```alphabet
const int MAX = 256
```

### 4.4 Scope

- **Global scope:** Top-level declarations are visible throughout the file.
- **Local scope:** Declarations inside blocks (`{ ... }`) are visible only
  within that block.
- **Shadowing:** A local variable may shadow a global variable of the same name
  within its scope.

### 4.5 Static Fields

Class fields may be declared `static` (`s`):

```alphabet
c Counter {
  s 5 count = 0
}
```

---

## 5. Expressions

### 5.1 Operator Precedence (highest to lowest)

| Precedence | Operators                | Associativity |
|------------|--------------------------|---------------|
| 1 (highest)| `()` `[]` `.` `?.`      | Left          |
| 2          | `-` `!` (unary)          | Right         |
| 3          | `*` `/` `%`              | Left          |
| 4          | `+` `-`                  | Left          |
| 5          | `<` `>` `<=` `>=`        | Left          |
| 6          | `==` `!=`                | Left          |
| 7          | `&&`                     | Left          |
| 8          | `\|\|`                   | Left          |
| 9          | `? :` (ternary)          | Right         |

### 5.2 Arithmetic

| Expression | Description                     |
|------------|---------------------------------|
| `a + b`    | Addition (numeric or string concat) |
| `a - b`    | Subtraction                     |
| `a * b`    | Multiplication                  |
| `a / b`    | Division                        |
| `a % b`    | Modulo                          |
| `-a`       | Unary negation                  |

When `+` involves a string operand, the result is string concatenation:

```alphabet
12 greeting = "Hello, " + "World!"   // "Hello, World!"
12 text = "count=" + 5               // "count=5"
12 text2 = 5 + " items"              // "5 items"
```

### 5.3 Comparison

| Expression | Description       |
|------------|-------------------|
| `a == b`   | Equal             |
| `a != b`   | Not equal         |
| `a < b`    | Less than         |
| `a > b`    | Greater than      |
| `a <= b`   | Less or equal     |
| `a >= b`   | Greater or equal  |

Comparisons between numeric types (int, float, bool) are supported with
implicit coercion.

### 5.4 Logical

| Expression | Description                    |
|------------|--------------------------------|
| `a && b`   | Logical AND (short-circuit)    |
| `a \|\| b` | Logical OR (short-circuit)    |
| `!a`       | Logical NOT                    |

### 5.5 Ternary

```alphabet
5 x = (score > 90) ? 1 : 0
```

### 5.6 String Interpolation (F-Strings)

F-strings allow embedding expressions inside string literals using `{expr}`:

```alphabet
5 age = 25
12 msg = f"Age is {age}, doubled is {age * 2}"
```

### 5.7 Range Expressions

The `..` operator creates a list of integers:

```alphabet
13 r = 1..5          // [1, 2, 3, 4]
13 r2 = 0..10        // [0, 1, 2, ..., 9]
```

### 5.8 Index Access

```alphabet
13 list = [10, 20, 30]
5 val = list[1]       // 20

14 map = {"a": 1}
5 val2 = map["a"]     // 1
```

Index assignment:

```alphabet
list[1] = 99
map["a"] = 42
```

### 5.9 Null-Safe Access

```alphabet
5 val = obj?.field    // returns null if obj is null, otherwise obj.field
```

---

## 6. Control Flow

### 6.1 If / Else

```alphabet
i (condition) {
  // then branch
} e {
  // else branch
}
```

Chained else-if:

```alphabet
i (score >= 90) {
  o("A")
} e i (score >= 80) {
  o("B")
} e {
  o("C")
}
```

Parentheses around the condition are **required**.

### 6.2 While Loop

```alphabet
5 i = 0
l (i < 10) {
  o(i)
  i = i + 1
}
```

### 6.3 Do-While Loop

```alphabet
5 i = 0
l (i < 10) {
  o(i)
  i = i + 1
} l { }   // syntax: do { ... } while (cond) — not available; use while
```

> **Note:** The implementation supports `do-while` via a special AST node, but
> the canonical syntax uses the `l (cond) { ... }` while-loop form.

### 6.4 For Loop

```alphabet
l (5 i = 0 : i < 5 : i = i + 1) {
  o(i)
}
```

The for-loop header contains three colon-separated parts: **initializer**,
**condition**, and **increment**.

### 6.5 For-Each Loop

Iterate over a list or map:

```alphabet
13 items = [1, 2, 3]
l (item : items) {
  o(item)
}

14 data = {"a": 1, "b": 2}
l (key : data) {
  o(key + "=" + data[key])
}
```

### 6.6 Break and Continue

```alphabet
l (true) {
  5 x = z.rand()
  i (x > 0.8) { b }       // break
  i (x < 0.2) { k }       // continue
  o(x)
}
```

Both `b` (break) and `k` (continue) exit/skip the **innermost** enclosing loop.
Nested break from an inner loop only breaks that inner loop.

### 6.7 Match (Pattern Matching)

```alphabet
5 x = 2
q (x) {
  1: o("one")
  2: o("two")
  3: o("three")
  e: o("other")    // default case
}
```

Each case is a **literal value** followed by `:` and a statement or block.
The `e:` (else) case is the default. Cases are evaluated top-to-bottom; the
first match wins.

---

## 7. Functions

### 7.1 Function Declaration

```alphabet
m RETURN_TYPE NAME(PARAM_LIST) {
  // body
  r EXPRESSION   // return
}
```

Where `RETURN_TYPE` is a type ID or named type, and `PARAM_LIST` is a
comma-separated list of `TYPE ID NAME` pairs.

```alphabet
m 5 add(5 a, 5 b) {
  r a + b
}

m 12 greet(12 name) {
  r "Hello, " + name + "!"
}

m void log_message(12 msg) {
  o(msg)
}
```

### 7.2 Calling Functions

```alphabet
5 result = add(3, 4)          // 7
12 greeting = greet("World")  // "Hello, World!"
```

### 7.3 Default Parameters

Functions may have default parameter values:

```alphabet
m 5 power(5 base, 5 exp = 2) {
  5 result = 1
  5 i = 0
  l (i < exp) {
    result = result * base
    i = i + 1
  }
  r result
}

o(power(3))      // 9  (uses default exp=2)
o(power(3, 3))   // 27
```

### 7.4 Recursive Functions

```alphabet
m 5 factorial(5 num) {
  i (num <= 1) { r 1 }
  r num * factorial(num - 1)
}
```

### 7.5 Function Scope

Top-level functions are **global**. Functions may call each other (forward
references are resolved at compile time). Functions may call themselves
recursively up to a call depth of 1000.

---

## 8. Classes

### 8.1 Class Declaration

```alphabet
c ClassName {
  // fields and methods
}
```

```alphabet
c Person {
  12 name = ""
  5 age = 0

  m void init(12 n, 5 a) {
    name = n
    age = a
  }

  m 12 greet() {
    r "Hi, I'm " + name
  }
}
```

### 8.2 Object Instantiation

```alphabet
15 p = n Person("Alice", 30)
o(p.greet())   // "Hi, I'm Alice"
```

The `n` keyword (`new`) creates a new object instance. The class name after `n`
must be a known class type.

### 8.3 Fields

- **Instance fields** are declared at the class body level with a type, name,
  and optional initializer.
- Fields are accessed via `object.field_name`.
- Fields are **public** by default.

```alphabet
c Point {
  5 px = 0
  5 py = 0
}

15 pt = n Point()
pt.px = 10
pt.py = 20
```

### 8.4 Methods

Methods are declared with the `m` keyword inside the class body, optionally
prefixed with a visibility modifier (`v` = public, `p` = private):

```alphabet
c Calculator {
  v m 5 add(5 a, 5 b) {
    r a + b
  }

  v m 5 multiply(5 a, 5 b) {
    r a * b
  }
}
```

### 8.5 Visibility Modifiers

| Modifier | Keyword | Description                          |
|----------|---------|--------------------------------------|
| Public   | `v`     | Accessible from outside the class    |
| Private  | `p`     | Accessible only within the class     |

Private methods/fields cannot be accessed from outside the class:

```alphabet
c Account {
  v 5 balance = 0

  v m 5 get_balance() { r balance }
  p m void add_interest() { balance = balance + balance * 0.05 }
}
```

### 8.6 Inheritance

Use the `^` keyword (`extends`) to inherit from a parent class:

```alphabet
c Animal {
  12 sound = "..."
  v m 12 speak() { r sound }
}

c Dog ^ Animal {
  v m 12 speak() { r "Woof!" }
}

c Cat ^ Animal {
  v m 12 speak() { r "Meow!" }
}

15 dog = n Dog()
o(dog.speak())   // "Woof!"
```

- Inherited fields and methods are available in the child class.
- Child classes may override parent methods.
- Multi-level inheritance is supported (grandchild → child → parent).

### 8.7 Static Members

The `s` keyword (`static`) declares class-level (not instance-level) members:

```alphabet
c Counter {
  s 5 count = 0

  s m 5 increment() {
    count = count + 1
    r count
  }
}

o(Counter.increment())   // 1
o(Counter.increment())   // 2
```

Static methods are called on the **class name**, not on an instance.

### 8.8 Abstract Classes

The `a` keyword (`abstract`) declares a class that cannot be instantiated
directly. Abstract classes may contain abstract methods (methods without a body):

```alphabet
a Shape {
  v m 5 area()          // abstract — no body
  v m 12 describe() {
    r "I am a shape"
  }
}

c Circle ^ Shape {
  5 radius = 0

  v m 5 area() {
    r 3.14159 * radius * radius
  }
}
```

- Abstract classes cannot be instantiated with `n`.
- Non-abstract subclasses must implement all abstract methods.

### 8.9 Interfaces

The `j` keyword (`interface`) declares a contract that classes may implement:

```alphabet
j Drawable {
  m void draw()
  m 12 name()
}

c Circle ^ Drawable {
  v m void draw() { o("Drawing circle") }
  v m 12 name() { r "Circle" }
}
```

Interfaces define method signatures without bodies. A class that implements an
interface must provide implementations for all interface methods.

---

## 9. Closures and Lambdas

### 9.1 Lambda Expressions

Lambdas are anonymous functions created with the `m` keyword:

```alphabet
m (args) {
  r expression
}
```

Examples:

```alphabet
m 5 adder(5 a) {
  m (5 b) {
    r a + b
  }
}

5 plus10 = adder(10)
o(plus10(5))   // 15
```

### 9.2 Closures

Lambdas capture variables from their enclosing scope:

```alphabet
5 multiplier = 10
m (5 x) {
  r x * multiplier
}
```

The lambda captures `multiplier` by reference, forming a closure.

### 9.3 Lambdas as Arguments

Lambdas can be passed to higher-order functions like `z.map`, `z.filter`,
and `z.reduce`:

```alphabet
13 nums = [1, 2, 3, 4, 5]
13 doubled = z.map(nums, m (5 x) { r x * 2 })
13 evens = z.filter(nums, m (5 x) { r x % 2 == 0 })
5 total = z.reduce(nums, 0, m (5 acc, 5 x) { r acc + x })
```

---

## 10. Built-in Functions

All built-in functions are accessed through the `z` namespace (system object).
Some have global aliases. Built-in functions are invoked as `z.FUNC_NAME(args)`.

### 10.1 I/O

| Function              | Description                              |
|-----------------------|------------------------------------------|
| `z.o(value)`         | Print value to stdout (with newline)     |
| `z.i()`              | Read a line from stdin                   |
| `z.t(message)`       | Throw a runtime exception               |

### 10.2 Type Conversion

| Function             | Description                              |
|----------------------|------------------------------------------|
| `z.tostr(value)`     | Convert any value to string              |
| `z.tonum(value)`     | Convert value to number                  |
| `z.type(value)`      | Return type name as string               |
| `z.is_null(value)`   | Returns 1 if null, 0 otherwise           |
| `z.is_empty(value)`  | Returns 1 if empty list/string/map/null  |

### 10.3 Math

| Function              | Description                              |
|-----------------------|------------------------------------------|
| `z.sqrt(x)`          | Square root                              |
| `z.abs(x)`           | Absolute value                           |
| `z.floor(x)`         | Floor                                    |
| `z.ceil(x)`          | Ceiling                                  |
| `z.round(x)`         | Round to nearest integer                 |
| `z.pow(base, exp)`   | Exponentiation                           |
| `z.min(a, b)`        | Minimum of two values                    |
| `z.max(a, b)`        | Maximum of two values                    |
| `z.log(x)`           | Natural logarithm                        |
| `z.log10(x)`         | Base-10 logarithm                        |
| `z.sin(x)`           | Sine                                     |
| `z.cos(x)`           | Cosine                                   |
| `z.tan(x)`           | Tangent                                  |
| `z.clamp(val, lo, hi)` | Clamp value to range [lo, hi]          |

### 10.4 String Operations

| Function                       | Description                          |
|--------------------------------|--------------------------------------|
| `z.len(str)`                  | Length (UTF-8 character count)       |
| `z.upper(str)`                | Uppercase                            |
| `z.lower(str)`                | Lowercase                            |
| `z.trim(str)`                 | Strip whitespace                     |
| `z.replace(str, old, new)`    | Replace all occurrences              |
| `z.split(str, delim)`         | Split into list                      |
| `z.join(list, sep)`           | Join list into string                |
| `z.substr(str, start, len)`   | Substring                            |
| `z.chr(code)`                 | Character from Unicode code point    |
| `z.ord(char)`                 | Unicode code point from character    |
| `z.starts_with(str, prefix)`  | Test prefix                          |
| `z.ends_with(str, suffix)`    | Test suffix                          |
| `z.contains(haystack, needle)`| Test containment (str or list)       |
| `z.find(haystack, needle)`    | Find index (-1 if not found)         |
| `z.count(haystack, needle)`   | Count occurrences                    |
| `z.reverse(str)`              | Reverse string                       |

### 10.5 List Operations

| Function                     | Description                          |
|------------------------------|--------------------------------------|
| `z.len(list)`               | Length of list                       |
| `z.append(list, value)`     | Append element                       |
| `z.pop_back(list)`          | Remove and return last element       |
| `z.insert(list, idx, val)`  | Insert at index                      |
| `z.remove(list, idx)`       | Remove element at index              |
| `z.contains(list, val)`     | Test containment                     |
| `z.find(list, val)`         | Find index of value                  |
| `z.count(list, val)`        | Count occurrences of value           |
| `z.reverse(list)`           | Reverse list (in-place)              |
| `z.sort(list)`              | Sort list (in-place)                 |
| `z.unique(list)`            | Remove duplicate elements            |
| `z.flatten(list)`           | Flatten nested lists                 |
| `z.slice(list, start)`      | Slice from start to end              |
| `z.slice(list, start, end)` | Slice from start to end (exclusive)  |
| `z.swap(list, i, j)`        | Swap elements at indices i and j     |
| `z.zip(list_a, list_b)`     | Zip two lists into pairs             |
| `z.enumerate(list)`         | Return list of [index, value] pairs  |
| `z.sum(list)`               | Sum of numeric elements              |
| `z.avg(list)`               | Average of numeric elements          |

### 10.6 Functional Operations

| Function                               | Description                      |
|----------------------------------------|----------------------------------|
| `z.map(list, lambda_fn)`              | Apply function to each element   |
| `z.filter(list, lambda_fn)`           | Keep elements where fn returns true |
| `z.reduce(list, init, lambda_fn)`     | Reduce list to single value      |

Lambda functions are passed as the string name of a named lambda:

```alphabet
13 doubled = z.map([1,2,3], m (5 x) { r x * 2 })
```

### 10.7 Map Operations

| Function           | Description                       |
|--------------------|-----------------------------------|
| `z.keys(map)`     | Return list of keys               |
| `z.values(map)`   | Return list of values             |
| `z.len(map)`      | Number of entries                 |
| `z.has(map, key)` | Test if key exists (returns 1/0)  |

### 10.8 Set Operations

| Function             | Description                        |
|----------------------|------------------------------------|
| `z.set()`           | Create empty set (backed by list)  |
| `z.add(set, val)`   | Add element (no duplicates)        |
| `z.has(set, val)`   | Test membership                    |
| `z.set_size(set)`   | Get set size                       |

### 10.9 String Builder

| Function                | Description                        |
|-------------------------|------------------------------------|
| `z.builder()`          | Create empty string builder (list) |
| `z.append_str(sb, str)`| Append string to builder           |
| `z.build(sb)`          | Convert builder to final string    |

### 10.10 Range

| Function                 | Description                           |
|--------------------------|---------------------------------------|
| `z.range(stop)`         | List [0..stop)                        |
| `z.range(start, stop)`  | List [start..stop)                    |
| `z.range(start, stop, step)` | List with step                   |

Maximum range size: 1,000,000 elements.

### 10.11 File I/O

| Function              | Description                              |
|-----------------------|------------------------------------------|
| `z.f(path)`          | Read entire file as string               |
| `z.fw(path, content)`| Write string to file (overwrite)         |
| `z.fa(path, content)`| Append string to file                    |
| `z.exists(path)`     | Check if file exists (returns 1/0)       |
| `z.file_size(path)`  | Get file size in bytes (-1 on error)     |

File operations are blocked in sandbox mode. Paths containing `..` or starting
with `/` are rejected for safety.

### 10.12 JSON

| Function                | Description                          |
|-------------------------|--------------------------------------|
| `z.json_parse(str)`    | Parse JSON string to value           |
| `z.json_stringify(val)`| Serialize value to JSON string       |

### 10.13 Random

| Function            | Description                              |
|---------------------|------------------------------------------|
| `z.rand()`          | Random float in [0.0, 1.0)             |
| `z.randint(lo, hi)` | Random integer in [lo, hi]              |

### 10.14 System / Process

| Function                 | Description                          |
|--------------------------|--------------------------------------|
| `z.exec(cmd)`           | Execute shell command, return output |
| `z.system(cmd)`         | Execute shell command, return code   |
| `z.args()`              | Get command-line arguments as list   |
| `z.exit(code)`          | Exit with status code                |
| `z.env(name)`           | Get environment variable             |
| `z.timestamp()`         | Current time in milliseconds         |
| `z.sleep(ms)`           | Sleep for N milliseconds (max 300s)  |

### 10.15 Networking

| Function              | Description                              |
|-----------------------|------------------------------------------|
| `z.http_get(url)`    | HTTP GET request, return response body   |
| `z.http_post(url, body)` | HTTP POST with JSON body             |

Network operations are blocked in sandbox mode.

### 10.16 Threading

| Function                | Description                          |
|-------------------------|--------------------------------------|
| `z.thread(lambda)`     | Create new thread, return thread ID  |
| `z.join(tid)`          | Wait for thread to finish            |
| `z.join_all()`         | Wait for all threads                 |
| `z.lock(name)`         | Create named mutex                   |
| `z.acquire(name)`      | Lock named mutex                     |
| `z.release(name)`      | Unlock named mutex                   |

### 10.17 Testing / Debug

| Function              | Description                              |
|-----------------------|------------------------------------------|
| `z.assert(cond)`     | Assert condition is truthy               |
| `z.assert(cond, msg)`| Assert with custom message               |
| `z.assert_eq(a, b)`  | Assert two values are equal              |

### 10.18 FFI (Foreign Function Interface)

| Function                       | Description                      |
|--------------------------------|----------------------------------|
| `z.dyn(lib_path, func, args...)` | Call native C function from shared library |

FFI is blocked in sandbox mode.

---

## 11. Error Handling

### 11.1 Try / Handle

```alphabet
t {
  // code that may throw
  z.t("something went wrong")
} h (12 err) {
  // handle exception — err contains the error value
  o("Caught: " + err)
}
```

The `t` keyword begins a try block. The `h` keyword begins a handler block.
The handler receives the exception value as a variable whose type is specified
by a type ID (typically `12` for string errors, or `15+` for custom objects).

### 11.2 Throwing Exceptions

Use `z.t(message)` to throw a runtime exception:

```alphabet
z.t("Invalid input")
z.t(42)   // exceptions can be any value
```

### 11.3 Nested Try/Handle

```alphabet
t {
  t {
    z.t("inner error")
  } h (12 err) {
    o("inner caught: " + err)
    z.t("re-thrown")     // re-throw from inner handler
  }
} h (12 err) {
  o("outer caught: " + err)
}
```

### 11.4 Exception Types

Exceptions can carry any `Value` type. The handler's type ID specifies what
type of exception it catches:

```alphabet
// Catch string errors
t { ... } h (12 err) { ... }

// Catch object errors
t { ... } h (15 err) { ... }
```

---

## 12. Import System

### 12.1 Basic Import

```alphabet
x "path/to/module.abc"
```

The `x` keyword imports all exports from the specified file into the current
scope.

### 12.2 Import with Alias

```alphabet
x "path/to/module.abc" as my_module
```

This imports the module under the given alias.

### 12.3 Export

Export names from a module using the `@` symbol:

```alphabet
@ my_function
@ MY_CONSTANT

m 5 my_function() { r 42 }
5 MY_CONSTANT = 100
```

### 12.4 Module Resolution

- Relative paths are resolved from the source file's directory.
- The `ALPHABET_PATH` environment variable specifies additional directories
  to search for imports.
- Compiled `.abc` bytecode files are loaded when available.

### 12.5 Example

```alphabet
# math_utils.abc
#alphabet<en>
@ factorial
@ PI

5 PI = 3.14159

m 5 factorial(5 n) {
  i (n <= 1) { r 1 }
  r n * factorial(n - 1)
}
```

```alphabet
# main.abc
#alphabet<en>
x "math_utils.abc"

o(factorial(5))   // 120
o(PI)              // 3.14159
```

---

## 13. Multilingual Keywords

Alphabet supports 5 human languages. Each keyword can be written in any
supported language. The lexer translates the human-language keyword to its
canonical single-letter form.

### 13.1 Keyword Mapping Table

| Canonical | Description  | English   | Amharic    | Spanish      | French         | German         |
|-----------|-------------|-----------|------------|--------------|----------------|----------------|
| `c`       | class       | class     | ክፍል        | clase        | classe         | klasse         |
| `a`       | abstract    | abstract  | ሥር         | abstracto    | abstrait       | abstrakt       |
| `j`       | interface   | interface | በይነገጽ     | interfaz     | interface      | schnittstelle  |
| `m`       | method      | method    | ዘዴ         | método       | méthode        | methode        |
| `i`       | if          | if        | ከሆነ        | si           | si             | wenn           |
| `e`       | else        | else      | ያለበለዚያ    | sino         | sinon          | sonst          |
| `l`       | loop        | loop      | ሉፕ         | bucle        | boucle         | schleife       |
| `r`       | return      | return    | ተመለስ       | retornar     | retour         | zurück         |
| `b`       | break       | break     | ስበር        | romper       | rompre         | brechen        |
| `k`       | continue    | continue  | ቀጥል        | continuar    | continuer      | fortsetzen     |
| `n`       | new         | new       | አዲስ        | nuevo        | nouveau        | neu            |
| `v`       | public      | public    | ግልጽ        | público      | public         | öffentlich     |
| `p`       | private     | private   | ግል         | privado      | privé          | privat         |
| `s`       | static      | static    | ቋሚ        | estático     | statique       | statisch       |
| `t`       | try         | try       | ሞክር        | intentar     | essayer        | versuchen      |
| `h`       | handle      | handle    | ያዟ         | capturar     | attraper       | fangen         |
| `z`       | output      | print     | ውጤት        | imprimir     | afficher       | ausgeben       |
| `z.i`     | input       | input     | ግብአት       | entrada      | entrer         | eingabe        |
| `x`       | import      | import    | አስገባ       | importar     | importer       | importieren    |
| `q`       | match       | match     | ምረጥ        | coincidir    | correspondre   | übereinstimmen |
| `^`       | extends     | extends   | ወራሽ       | extiende     | étend          | erweitert      |
| `@`       | export      | export    | ላክ         | exportar     | exporter       | exportieren    |
| `\x80`    | const       | const     | ቋሚ-እሴት     | constante    | constante      | konstante      |

### 13.2 How Translation Works

1. The lexer reads the `#alphabet<LANG>` header to determine the language.
2. When scanning an identifier, the lexer checks if it matches a keyword in
   the specified language.
3. If a match is found, the token type is set to the corresponding `TokenType`
   enum value (e.g., `TokenType::IF`, `TokenType::CLASS`).
4. The parser operates on the canonical token types regardless of source
   language.

This means the same program can be written in any of the 5 languages with
identical behavior.

---

## 14. Bytecode VM Architecture

### 14.1 Overview

Alphabet uses a **stack-based bytecode virtual machine** implemented in C++17.
The compilation pipeline is:

```
Source Code (.abc)
    → Lexer (tokenization + keyword translation)
    → Parser (AST construction)
    → Compiler (AST → bytecode)
    → VM (bytecode execution)
```

### 14.2 Value Representation

All runtime values are represented as a tagged union (`Value`):

```cpp
variant<monostate, int64_t, double, bool, string, shared_ptr<List>,
        shared_ptr<Map>, ObjectPtr>
```

- `monostate` → null
- `int64_t` → integer
- `double` → float
- `bool` → boolean
- `string` → string
- `shared_ptr<List>` → list (vector of Values)
- `shared_ptr<Map>` → map (unordered_map of string → Value)
- `ObjectPtr` → class instance

### 14.3 Call Frames

Each function call pushes a `CallFrame` onto the call stack:

```cpp
struct CallFrame {
    const vector<Instruction> *bytecode;  // bytecode to execute
    size_t ip;                             // instruction pointer
    unordered_map<string, Value> locals;   // local variables
    vector<pair<size_t, size_t>> try_stack; // exception handler stack
};
```

- Maximum call depth: **1000** (stack overflow error if exceeded).
- Maximum stack size: **65536** entries.

### 14.4 Program Structure

A compiled program (`Program`) contains:

| Field          | Description                                     |
|----------------|-------------------------------------------------|
| `version`      | Bytecode format version (currently 1)           |
| `main`         | Main bytecode instruction sequence              |
| `static_init`  | Static initialization bytecode                  |
| `classes`      | Map of class ID → compiled class definition     |
| `functions`    | Map of function name → compiled method          |
| `globals`      | List of global variable names                   |
| `constant_pool`| Shared pool of constant values                  |

### 14.5 Compiled Class

```cpp
struct CompiledClass {
    string name;
    string superclass;
    uint16_t id;                         // type ID (≥15)
    bool is_abstract;
    unordered_map<string, CompiledMethod> methods;
    unordered_map<string, CompiledMethod> static_methods;
    vector<Instruction> static_init;     // static field initializers
    vector<Instruction> field_init;      // instance field initializers
    unordered_set<string> private_fields;
    unordered_set<string> private_methods;
};
```

### 14.6 Compiled Method

```cpp
struct CompiledMethod {
    vector<Instruction> bytecode;
    vector<string> param_names;
    vector<vector<Instruction>> default_value_bytecodes;
};
```

### 14.7 Opcodes

Each instruction is an `OpCode` byte optionally followed by an operand.

| Opcode              | Value | Operand              | Description                     |
|---------------------|-------|----------------------|---------------------------------|
| `PUSH_CONST`        | 1     | constant index       | Push constant onto stack        |
| `LOAD_VAR`          | 2     | variable index       | Load global variable            |
| `STORE_VAR`         | 3     | variable index       | Store to global variable        |
| `LOAD_FIELD`        | 4     | field name           | Load object field               |
| `STORE_FIELD`       | 5     | field name           | Store to object field           |
| `ADD`               | 6     | —                    | Add / concatenate               |
| `SUB`               | 7     | —                    | Subtract                        |
| `MUL`               | 8     | —                    | Multiply                        |
| `DIV`               | 9     | —                    | Divide                          |
| `PERCENT`           | 10    | —                    | Modulo                          |
| `EQ`                | 11    | —                    | Equal                           |
| `NE`                | 12    | —                    | Not equal                       |
| `GT`                | 13    | —                    | Greater than                    |
| `GE`                | 14    | —                    | Greater or equal                |
| `LT`                | 15    | —                    | Less than                       |
| `LE`                | 16    | —                    | Less or equal                   |
| `AND`               | 17    | —                    | Logical AND                     |
| `OR`                | 18    | —                    | Logical OR                      |
| `NOT`               | 19    | —                    | Logical NOT                     |
| `JUMP`              | 20    | target offset        | Unconditional jump              |
| `JUMP_IF_FALSE`     | 21    | target offset        | Jump if top is false            |
| `JUMP_IF_TRUE`      | 41    | target offset        | Jump if top is true             |
| `CALL`              | 22    | argument count       | Call function/method            |
| `RET`               | 23    | —                    | Return from function            |
| `NEW`               | 24    | class ID             | Create new instance             |
| `POP`               | 25    | —                    | Pop stack top                   |
| `PRINT`             | 26    | —                    | Print value (legacy)            |
| `HALT`              | 27    | —                    | Stop execution                  |
| `SETUP_TRY`         | 28    | handler offset       | Push exception handler          |
| `POP_TRY`           | 29    | —                    | Pop exception handler           |
| `THROW`             | 30    | —                    | Throw exception                 |
| `GET_STATIC`        | 31    | class ID + field     | Load static field               |
| `SET_STATIC`        | 32    | class ID + field     | Store static field              |
| `BUILD_LIST`        | 33    | element count        | Build list from stack           |
| `BUILD_MAP`         | 34    | entry count          | Build map from stack            |
| `LOAD_INDEX`        | 35    | —                    | Index into list/map             |
| `STORE_INDEX`       | 36    | —                    | Store to index                  |
| `DUP`               | 37    | —                    | Duplicate stack top             |
| `LOOP_START`        | 38    | —                    | Loop start marker               |
| `BREAK_JUMP`        | 39    | target offset        | Break from loop                 |
| `CONTINUE_JUMP`     | 40    | target offset        | Continue loop                   |
| `MARK_CONST`        | 50    | variable index       | Mark variable as constant       |
| `NOP`               | 51    | —                    | No operation                    |
| `PUSH_CONST_POOL`   | 52    | pool index           | Push from constant pool         |
| `LOAD_SUPER`        | 53    | —                    | Load superclass reference       |

### 14.8 Execution Model

1. The VM initializes the program by loading globals, classes, and functions.
2. Static initializers are executed first.
3. The `main` bytecode sequence is executed instruction by instruction.
4. Each instruction is fetched, decoded, and executed.
5. The VM maintains a value stack, a call frame stack, and a try/catch stack.
6. Execution halts when `HALT` is reached, an unhandled exception occurs, or
   `z.exit()` is called.

### 14.9 Binary File Format

Compiled bytecode can be saved to `.abc` binary files with a header:

| Offset | Size    | Field       | Description                |
|--------|---------|-------------|----------------------------|
| 0      | 4 bytes | Magic       | `"ABCM"` magic number      |
| 4      | 2 bytes | Version     | Bytecode format version    |
| 6      | varies  | Data        | Serialized program data    |

---

## 15. Grammar (EBNF)

The following is the formal Extended BNF grammar for Alphabet.

### Notation

- `::=` defines a production
- `|` separates alternatives
- `[ ]` denotes optional
- `{ }` denotes zero or more
- Terminals are in `"quotes"` or `UPPER_CASE`
- Non-terminals are in *italics*

### Program

```
program         ::= HEADER declaration*

HEADER          ::= "#alphabet" "<" LANG_CODE ">"
LANG_CODE       ::= "en" | "am" | "es" | "fr" | "de"
```

### Declarations

```
declaration     ::= import_decl
                  | export_decl
                  | var_decl
                  | const_decl
                  | func_decl
                  | class_decl
                  | abstract_class_decl
                  | interface_decl
                  | statement

import_decl     ::= IMPORT_KW STRING
                  | IMPORT_KW STRING "as" IDENT

export_decl     ::= "@" IDENT ( "," IDENT )*

class_decl      ::= CLASS_KW IDENT [ "^" IDENT ] "{" class_body "}"

abstract_class_decl ::= ABSTRACT_KW IDENT [ "^" IDENT ] "{" class_body "}"

interface_decl  ::= INTERFACE_KW IDENT "{" interface_body "}"

class_body      ::= { class_member }
class_member    ::= [ access_mod ] member_decl
member_decl     ::= var_decl
                  | func_decl
                  | static_decl

access_mod      ::= PUBLIC_KW | PRIVATE_KW

static_decl     ::= STATIC_KW ( var_decl | func_decl )

interface_body  ::= { func_sig ";" }
```

### Variables

```
var_decl        ::= type_expr IDENT "=" expression
                  | type_expr IDENT
                  | LET_KW IDENT "=" expression

const_decl      ::= CONST_KW type_expr IDENT "=" expression

type_expr       ::= type_id
                  | IDENT
                  | type_id "[" "]"

type_id         ::= "0" | "1" | "2" | "3" | "4" | "5"
                  | "6" | "7" | "8" | "9" | "10" | "11"
                  | "12" | "13" | "14"
                  | "int" | "float" | "bool" | "str"
                  | "list" | "map" | "void"
                  | "i8" | "i16" | "i32" | "i64"
                  | "f32" | "f64" | "dec" | "cpx"
```

### Functions

```
func_decl       ::= METHOD_KW type_expr IDENT "(" [ param_list ] ")" block
                  | METHOD_KW IDENT "(" [ param_list ] ")" block

func_sig        ::= METHOD_KW type_expr IDENT "(" [ param_list ] ")"

param_list      ::= parameter ( "," parameter )*
parameter       ::= type_expr IDENT [ "=" expression ]
```

### Statements

```
statement       ::= block
                  | if_stmt
                  | loop_stmt
                  | for_stmt
                  | return_stmt
                  | break_stmt
                  | continue_stmt
                  | try_stmt
                  | match_stmt
                  | expression_stmt

block           ::= "{" { statement } "}"

if_stmt         ::= IF_KW "(" expression ")" block [ ELSE_KW ( if_stmt | block ) ]

loop_stmt       ::= LOOP_KW "(" expression ")" block

for_stmt        ::= LOOP_KW "(" var_decl ":" expression ":" expression ")" block
                  | LOOP_KW "(" IDENT ":" expression ")" block

return_stmt     ::= RETURN_KW [ expression ]
break_stmt      ::= BREAK_KW
continue_stmt   ::= CONTINUE_KW

try_stmt        ::= TRY_KW block HANDLE_KW "(" type_expr IDENT ")" block

match_stmt      ::= MATCH_KW "(" expression ")" "{" { match_case } [ default_case ] "}"
match_case      ::= expression ":" ( block | expression_stmt )
default_case    ::= "e" ":" ( block | expression_stmt )

expression_stmt ::= expression
```

### Expressions

```
expression      ::= ternary_expr

ternary_expr    ::= or_expr [ "?" expression ":" expression ]

or_expr         ::= and_expr { "||" and_expr }
and_expr        ::= equality { "&&" equality }

equality        ::= comparison { ( "==" | "!=" ) comparison }
comparison      ::= addition { ( "<" | ">" | "<=" | ">=" ) addition }
addition        ::= multiply { ( "+" | "-" ) multiply }
multiply        ::= unary { ( "*" | "/" | "%" ) unary }

unary           ::= ( "-" | "!" ) unary | postfix

postfix         ::= primary { call_suffix | index_suffix | field_suffix
                             | null_safe_suffix }

call_suffix     ::= "(" [ arg_list ] ")"
index_suffix    ::= "[" expression "]"
field_suffix    ::= "." IDENT
null_safe_suffix ::= "?." IDENT

arg_list        ::= expression ( "," expression )*

primary         ::= INTEGER
                  | FLOAT
                  | STRING
                  | FSTRING
                  | RAW_STRING
                  | "true" | "false"
                  | "null"
                  | IDENT
                  | "(" expression ")"
                  | list_literal
                  | map_literal
                  | range_expr
                  | lambda_expr
                  | new_expr

list_literal    ::= "[" [ expression ( "," expression )* [ "," ] ] "]"

map_literal     ::= "{" [ map_entry ( "," map_entry )* [ "," ] ] "}"
map_entry       ::= expression ":" expression

range_expr      ::= expression ".." expression

lambda_expr     ::= METHOD_KW "(" [ param_list ] ")" block

new_expr        ::= NEW_KW IDENT "(" [ arg_list ] ")"
```

### Literals and Identifiers

```
INTEGER         ::= DIGIT+ { "_" DIGIT+ }
FLOAT           ::= DIGIT+ "." DIGIT+ [ ( "e" | "E" ) [ "+" | "-" ] DIGIT+ ]
STRING          ::= '"' { CHAR | ESCAPE } '"'
                  | "'" { CHAR | ESCAPE } "'"
FSTRING         ::= 'f"' { FSTRING_PART | CHAR | ESCAPE } '"'
RAW_STRING      ::= 'r"' { CHAR } '"'
BOOLEAN         ::= "true" | "false"

CHAR            ::= any Unicode character except unescaped '"' or "'"
ESCAPE          ::= "\" ( "n" | "r" | "t" | "b" | "\" | '"' | "'" )
FSTRING_PART    ::= "{" expression "}"

IDENT           ::= ( LETTER | "_" ) { LETTER | DIGIT | "_" }
LETTER          ::= [a-zA-Z] | Unicode letter
DIGIT           ::= [0-9]
```

### Comments

```
comment         ::= single_line_comment | multi_line_comment
single_line_comment ::= "//" { any character except newline }
multi_line_comment  ::= "/*" { any character } "*/"
```

---

*End of Alphabet Language Specification v2.3.5*
