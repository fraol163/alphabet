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

Alphabet is a beginner-friendly programming language with 19 single-letter keywords. It was designed from the ground up to make programming accessible to everyone, regardless of their native language or prior experience.

The language compiles to bytecode and runs on a stack-based virtual machine, both written entirely in C++17. This means there are no runtime dependencies -- you get a single binary that just works.

### Design Philosophy

The core philosophy behind Alphabet is that the syntax of a programming language should never be the barrier to learning. Most languages force beginners to memorize dozens of keywords, understand complex type declarations, and fight with semicolons and indentation rules before they can write their first useful program. Alphabet removes all of that.

Instead of writing `int`, `string`, `List<Integer>`, or `HashMap<String, Object>`, you write numbers. The number 5 means integer. The number 12 means string. The number 13 means list. The number 14 means map. This is not arbitrary -- the numbers correspond to a type hierarchy that the compiler understands, but beginners only need to remember four numbers to get started.

Instead of writing `if`, `else`, `while`, `return`, `class`, `public`, `private`, `static`, `try`, `catch`, `import`, and `match`, you write single letters. The letter i means if. The letter e means else. The letter l means loop. The letter r means return. Nineteen letters cover every concept in the language.

### Who Is It For?

Alphabet is designed primarily for students who are learning programming for the first time. It is also useful for educators who want to teach computational thinking without syntax overhead, for prototypers who want to test ideas quickly, and for developers building educational tools.

It is not designed to replace Python, Java, or C for production software. It is designed to be the best first language, so that when students graduate to production languages, they already understand variables, functions, loops, classes, exceptions, and collections.

---

## 2. Getting Started

### Installation

Installing Alphabet takes one command and about ten seconds. Open your terminal and run:

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

This command downloads an installer script that detects your operating system (Linux, macOS, or Windows via WSL), detects your processor architecture (x86_64 or ARM64), downloads the correct pre-built binary from GitHub Releases, places it in your home directory under .local/bin, and adds that directory to your PATH if it is not already there. You do not need root access or sudo.

If a pre-built binary is not available for your platform, the installer automatically falls back to building from source, which requires a C++17 compiler and CMake.

### Your First Program

Create a file called hello.abc with this content:

```
#alphabet<en>
12 greeting = "Hello, Alphabet!"
z.o(greeting)
```

Line one is the magic header. Every Alphabet file must start with a line containing a hash symbol, the word alphabet, and a language code in angle brackets. The language code tells the compiler which set of keywords you are using. English is en.

Line two declares a variable. The number 12 is the type ID for strings. The variable is named greeting and is assigned the value "Hello, Alphabet!".

Line three calls the built-in output function. The z namespace contains all built-in functions. The o function prints its argument to standard output.

Run the program with:

```bash
alphabet hello.abc
```

You should see `Hello, Alphabet!` printed to your terminal.

### The Magic Header

The magic header is required as the first line of every source file. It tells the compiler two things: that this is an Alphabet source file, and which language you are writing keywords in.

Valid headers include:
- `#alphabet<en>` for English
- `#alphabet<am>` for Amharic
- `#alphabet<es>` for Spanish
- `#alphabet<fr>` for French
- `#alphabet<de>` for German

The header is not a comment -- it is a compiler directive. If you forget it, the compiler will give you a clear error message telling you to add it.

---

## 3. Language Fundamentals

### Comments

Comments start with two forward slashes and extend to the end of the line. There are no block comments.

```
// This is a comment
5 x = 10  // This is also a comment
```

### Identifiers

Names for variables and functions can use ASCII letters, digits (not at the start), underscores, and any UTF-8 characters. This means you can write variable names in Amharic, Chinese, Russian, Arabic, or any other script.

```
5 count = 10
5 _temp = 20
5 ቁጥር = 30      // Amharic variable name
5 数字 = 40      // Chinese variable name
```

### File Extension

All Alphabet source files use the .abc extension. This is a convention, not a requirement -- the compiler will process any file regardless of extension -- but using .abc helps editors and tools identify the language.

---

## 4. Data Types

Alphabet uses numeric type IDs instead of named types. This is one of the language's most distinctive features. Instead of writing `int` or `string`, you write a number that represents the type.

### Common Types

Most programs only need four type IDs:

- **5** for integers and general numbers
- **12** for strings (text)
- **13** for lists (ordered collections)
- **14** for maps (key-value collections)

### Full Type System

The complete type system includes more specific numeric types:

- Types 1 through 4 are specific integer sizes (8-bit, 16-bit, 32-bit, 64-bit)
- Type 5 is the generic integer type
- Types 6 and 7 are specific float sizes (32-bit and 64-bit)
- Type 8 is the generic float type
- Type 11 is boolean (represented as 0 or 1)
- Types 15 and above are custom class types

In practice, beginners should use type 5 for all numbers, type 12 for text, type 13 for lists, and type 14 for maps. The more specific types are available for advanced users who need them.

---

## 5. Variables

### Declaration

To declare a variable, write the type ID, the variable name, an equals sign, and the value:

```
5 age = 25
12 name = "Fraol"
13 scores = [95, 87, 92]
```

### Reassignment

Variables can be reassigned at any time. You write the type ID again, the variable name, and the new value:

```
5 x = 10
5 x = x + 5     // x is now 15
```

The type ID must match the original declaration. You cannot declare a variable as type 5 and later assign a string to it.

### Constants

There is no separate const keyword in the language. By convention, uppercase variable names indicate values that should not be changed:

```
5 MAX_RETRIES = 3
12 APP_NAME = "MyApp"
```

---

## 6. Operators

### Arithmetic

The standard arithmetic operators work on numbers:

- Addition: `+`
- Subtraction: `-`
- Multiplication: `*`
- Division: `/` (integer division for integers, floating point for floats)
- Modulo: `%` (remainder after division)

Standard operator precedence applies: multiplication and division are evaluated before addition and subtraction. Use parentheses to change the order.

### Comparison

Comparison operators return 1 for true and 0 for false:

- Equal: `==`
- Not equal: `!=`
- Greater than: `>`
- Less than: `<`
- Greater than or equal: `>=`
- Less than or equal: `<=`

### Logical

Logical operators work on the numeric truthiness of values. Zero is false, anything else is true:

- Logical AND: `&&`
- Logical OR: `||`
- Logical NOT: `!`

### String Concatenation

The plus operator concatenates strings. When one operand is a string and the other is a number, the number is automatically converted to a string:

```
12 s = "Hello" + " " + "World"  // "Hello World"
12 t = "Count: " + 42           // "Count: 42"
```

---

## 7. Control Flow

### If/Else

Conditional execution uses the letter i for if and the letter e for else. The condition goes in parentheses, and the body goes in curly braces:

```
5 temperature = 35
i (temperature > 30) {
  z.o("Hot day")
} e {
  z.o("Nice day")
}
```

You can chain conditions by writing another i after the closing brace:

```
i (score >= 90) {
  z.o("A")
} i (score >= 80) {
  z.o("B")
} e {
  z.o("C")
}
```

### While Loop

The letter l creates a loop that continues while a condition is true:

```
5 count = 0
l (count < 5) {
  z.o(count)
  5 count = count + 1
}
```

### For Loop

A C-style for loop is also available. Write l followed by parentheses containing three parts separated by colons: the initializer, the condition, and the increment:

```
l (5 i = 0 : i < 10 : i = i + 1) {
  z.o(i)
}
```

### Break and Continue

Inside a loop, the letter b breaks out of the loop immediately. The letter k skips to the next iteration:

```
l (5 i = 0 : i < 100 : i = i + 1) {
  i (i == 5) { b }       // stop at 5
  i (i % 2 == 0) { k }   // skip even numbers
  z.o(i)
}
```

---

## 8. Functions

### Defining Functions

The letter m defines a function. After m, write the return type, the function name, and the parameters in parentheses. Each parameter has a type and a name:

```
m 5 add(5 a, 5 b) {
  r a + b
}
```

This defines a function named add that takes two integer parameters, returns an integer, and returns their sum. The letter r means return.

### Calling Functions

Call a function by writing its name followed by arguments in parentheses:

```
5 result = add(3, 4)    // result is 7
z.o(result)
```

### Recursion

Functions can call themselves. Here is a recursive factorial:

```
m 5 factorial(5 num) {
  i (num <= 1) { r 1 }
  r num * factorial(num - 1)
}
z.o(factorial(5))  // 120
```

The virtual machine has a maximum call depth of 1000 to prevent stack overflow from infinite recursion.

---

## 9. Classes and OOP

### Defining a Class

The letter c defines a class. Inside the class body, you declare fields with type IDs and method names, and define methods with the letter m:

```
c Person {
  12 name = ""
  5 age = 0

  v m 12 greet() {
    r "Hi, I'm " + name
  }
}
```

The letter v before m means the method is public. Without v, the method is accessible within the class only.

### Creating Objects

The letter n creates a new instance of a class. Custom classes start at type ID 15:

```
15 p = n Person()
12 msg = p.greet()
```

### Visibility

Three visibility modifiers are available:

- **v** (public) -- accessible from anywhere
- **p** (private) -- accessible only within the class
- **s** (static) -- belongs to the class, not to instances

### Inheritance

The caret symbol creates an inheritance relationship:

```
c Animal {
  v m 12 speak() { r "..." }
}

c Dog ^ Animal {
  v m 12 speak() { r "Woof!" }
}
```

Dog inherits from Animal and overrides the speak method. When you call speak on a Dog instance, it returns "Woof!" instead of "...".

### Abstract Classes and Interfaces

The letter a before c defines an abstract class that cannot be instantiated directly. The letter j defines an interface -- a contract that classes can implement:

```
a c Shape {
  m 5 area()       // abstract method, no body
}

j Printable {
  m 12 to_string()
}
```

---

## 10. Collections

### Lists

Lists are ordered collections of values. They use type ID 13:

```
13 fruits = ["apple", "banana", "cherry"]
13 numbers = [10, 20, 30, 40, 50]
```

Access elements by index in square brackets. Indexing starts at zero. Negative indices count from the end:

```
z.o(numbers[0])     // 10 (first element)
z.o(numbers[-1])    // 50 (last element)
z.o(numbers[-2])    // 40 (second to last)
```

Get the length with z.len, append with z.append, remove the last element with z.pop_back, and check membership with z.contains.

### Maps

Maps store key-value pairs. They use type ID 14:

```
14 config = {"host": "localhost", "port": 8080}
```

Access values by key in square brackets. Add new entries the same way:

```
z.o(config["host"])      // "localhost"
config["timeout"] = 30   // add new entry
```

Get all keys with z.keys and all values with z.values. Both return a list.

### Ranges

The z.range function generates a list of numbers:

- z.range(5) produces [0, 1, 2, 3, 4]
- z.range(2, 7) produces [2, 3, 4, 5, 6]
- z.range(0, 10, 2) produces [0, 2, 4, 6, 8]

This is useful for iterating over indices or generating sequences.

---

## 11. String Operations

Alphabet has over 20 built-in string functions accessible through the z namespace.

### Case Conversion

z.upper converts to uppercase, z.lower converts to lowercase.

### Trimming

z.remove whitespace from the beginning and end of a string.

### Splitting and Joining

z.split breaks a string into a list using a delimiter. z.join puts a list back together with a separator between elements. These two functions are inverses of each other.

### Searching

z.contains checks if a substring exists anywhere in the string. z.starts_with checks the beginning. z.ends_with checks the end. All return 1 for true and 0 for false.

### Replacing

z.replace finds all occurrences of a substring and replaces them with another string.

### Extracting

z.substr extracts a portion of a string starting at a given index, with an optional length.

### Character Conversion

z.chr converts a number to its ASCII character. z.ord does the reverse -- it gives you the numeric code of a character.

---

## 12. Exception Handling

Exception handling uses t for try and h for handle (the equivalent of catch):

```
t {
  5 result = 10 / 0
} h (12 e) {
  z.o("Error: " + e)
}
```

If an error occurs inside the try block, execution jumps to the handle block. The variable after h receives the error message. The type of the exception variable is always string (type 12).

To throw an error yourself, call z.t with an optional message:

```
m 5 divide(5 a, 5 b) {
  i (b == 0) { z.t("Division by zero") }
  r a / b
}
```

---

## 13. Pattern Matching

The letter q provides pattern matching, similar to switch in C or match in Rust:

```
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

The compiler evaluates the expression in parentheses and compares it against each case value. The first matching case runs. The e case is the default -- it runs if nothing else matched. This is cleaner than a long if-else chain when you have many discrete values to check.

---

## 14. Module System

### Importing Files

The letter x imports another Alphabet source file. The path goes in quotes:

```
x "stdlib/math.abc"
```

Once imported, all public functions and variables from that file are available in the current file.

### Search Paths

The compiler searches for imports relative to the source file first. If not found, it checks directories listed in the ALPHABET_PATH environment variable. You can set this to a colon-separated list of directories.

### Writing Modules

Any .abc file can be a module. Just define functions in it with public visibility and they become available to anyone who imports the file.

---

## 15. Built-In Functions

All built-in functions are accessed through the z namespace. Here is the complete list organized by category.

### I/O Functions

z.o prints a value to standard output with a newline. z.i reads a line of input from the user and returns it as a string (or number if the input looks numeric). z.f reads the contents of a file and returns it as a string.

### Math Functions

z.sqrt computes the square root. z.abs returns the absolute value. z.pow raises a number to a power. z.floor rounds down to the nearest integer. z.ceil rounds up to the nearest integer. z.sin and z.cos compute trigonometric functions.

### Type Functions

z.type returns the type of a value as a string -- "number", "string", "null", or "object". z.len returns the length of a string, list, or map. z.tostr converts any value to its string representation. z.tonum converts a string to a number, returning 0 if the conversion fails.

### String Functions

z.upper and z.lower change case. z.trim removes whitespace. z.split and z.join convert between strings and lists. z.replace substitutes substrings. z.substr extracts portions. z.chr and z.ord convert between characters and codes. z.starts_with, z.ends_with, and z.contains perform searches. z.reverse flips a string.

### List and Map Functions

z.append adds an element to the end of a list. z.pop_back removes and returns the last element. z.contains checks membership. z.reverse returns a new reversed copy. z.range generates number sequences. z.keys and z.values extract from maps.

### Error Functions

z.t throws an exception with an optional message.

### FFI Function

z.dyn calls a function in a native C shared library. It takes the library path, function name, and up to four integer arguments.

---

## 16. Standard Library

Alphabet ships with four standard library modules in the stdlib directory.

### math.abc

This module provides mathematical utility functions: factorial for computing n factorial, gcd for greatest common divisor, lcm for least common multiple, max and min for comparisons, clamp for constraining a value to a range, is_even and is_odd for parity checks, and sign for determining if a number is positive, negative, or zero.

### io.abc

This module wraps the built-in I/O functions: print for output without a newline, println for output with a newline, and read_file for reading file contents.

### string.abc

This module provides named functions for all string operations so you can call them without the z prefix: contains, starts_with, ends_with, split, join, replace, trim, upper, lower, substr, slice, reverse, length, chr, and ord.

### list.abc

This module provides named functions for list and map operations: length, push, pop, contains, reverse, first, last, range, range_from, range_step, keys, and values.

---

## 17. Multilingual Keywords

Alphabet supports writing code in five natural languages. The compiler translates keywords from your chosen language to the same internal representation, so programs written in different languages produce identical bytecode.

### Supported Languages

English (en), Amharic (am), Spanish (es), French (fr), and German (de).

### How It Works

Set the language in the magic header. Then write keywords in that language. The compiler looks up each word in its translation table and converts it to the corresponding single-letter token.

### UTF-8 Variable Names

Variable names can use any Unicode characters. This means you can write variable names in Amharic, Chinese, Russian, Arabic, or any other script alongside English keywords, or use non-English keywords with English variable names, or mix freely.

---

## 18. FFI - Native C Integration

Alphabet can call functions in native C shared libraries at runtime using z.dyn. This lets you extend the language with existing C libraries for graphics, networking, or any other functionality.

The function takes the path to a shared library (.so on Linux, .dylib on macOS, .dll on Windows), the name of the function to call, and up to four integer arguments. It returns the function's return value as a number.

In sandbox mode (--sandbox flag), FFI calls are blocked for security.

---

## 19. REPL Mode

The REPL (Read-Eval-Print Loop) lets you type code interactively and see results immediately. Start it with alphabet --repl.

Key features of the REPL:

- **Persistent state:** Variables and functions you define stay defined across lines. You can build up a program piece by piece.
- **Multi-line input:** If you open a curly brace, the REPL waits for you to close it before executing.
- **Error recovery:** If you make a mistake, the REPL shows the error and keeps your previous state. You do not lose your work.
- **History:** Your input is saved to ~/.alphabet_history. Type "history" to see recent entries. Type "!!" to repeat the last command.
- **Exit:** Type q, quit, or exit to leave the REPL.

---

## 20. Debugging

Alphabet includes a built-in debugger. Run a program with alphabet --debug to activate it.

### Setting Breakpoints

Use add_break followed by a line number to set a breakpoint. Use del_break to remove one. Use breakpoints to list all active breakpoints.

### Stepping Through Code

When the program hits a breakpoint, it stops and shows a JSON event with the line number and reason. You can then type commands: continue to resume, step to execute one line and stop again, locals to see local variables, globals to see global variables, stack to see the call stack, and print to see the operand stack.

### Debugger Output

The debugger outputs structured JSON, which means external tools can parse the output and build graphical debugger interfaces on top of it.

---

## 21. Command Line Interface

The alphabet command supports several modes:

- **Run a file:** alphabet program.abc
- **Interactive REPL:** alphabet --repl
- **LSP server:** alphabet --lsp
- **Debug mode:** alphabet --debug program.abc
- **Inspect bytecode:** alphabet --dump-bytecode program.abc
- **Sandbox mode:** alphabet --sandbox program.abc
- **Compile only:** alphabet -c program.abc
- **Self-update:** alphabet update
- **Version:** alphabet --version
- **Help:** alphabet --help

The ALPHABET_PATH environment variable can be set to a colon-separated list of directories to search for imports.

---

## 22. Architecture

Alphabet follows a classic four-stage compiler pipeline.

The lexer reads source code character by character and produces a stream of tokens. It handles multilingual keyword translation, UTF-8 identifiers, string escape sequences, and comments. It requires the magic header on line one.

The parser takes the token stream and builds an abstract syntax tree using recursive descent parsing. It produces clear error messages with line numbers, column numbers, and source context.

The compiler walks the AST and emits bytecode instructions. It performs type checking at compile time using the numeric type IDs. It resolves class inheritance, method calls, and import paths.

The virtual machine executes the bytecode on a stack-based interpreter with 43 opcodes. It manages a dynamic stack (starts at 4096 slots, doubles as needed), a call frame stack for function calls, a global variable table, and an exception handling mechanism.

Additional modules include the FFI bridge for calling native C functions and the LSP server for editor integration.

---

## 23. Type System Reference

The type system uses numeric IDs from 0 to infinity:

- 0 is void (no value)
- 1 through 4 are specific integer sizes
- 5 is the generic integer type (most common)
- 6 and 7 are specific float sizes
- 8 is the generic float type
- 9 and 10 are decimal and complex (reserved)
- 11 is boolean
- 12 is string
- 13 is list
- 14 is map
- 15 and above are custom class types, assigned sequentially as classes are defined

At runtime, all numeric types are stored as 64-bit floating point. Strings are stored as C++ std::string. Lists and maps use shared pointers to heap-allocated containers for efficient passing by reference.

---

## 24. Complete Keyword Reference

| Token | English | Description |
|-------|---------|-------------|
| a | abstract | Abstract class or method |
| b | break | Exit current loop |
| c | class | Define a class |
| e | else | Else branch of condition |
| h | handle | Catch exception (like catch) |
| i | if | Conditional statement |
| j | interface | Interface definition |
| k | continue | Skip to next loop iteration |
| l | loop | Loop (while or for) |
| m | method | Define a function |
| n | new | Create a new instance |
| p | private | Private access modifier |
| r | return | Return from function |
| s | static | Static member |
| t | try | Try block for exceptions |
| v | public | Public access modifier |
| z | system | System namespace (built-ins) |
| x | import | Import a module |
| q | match | Pattern matching |

Additionally, the caret symbol (^) means extends (inheritance), and the at symbol (@) is reserved for future export functionality.

---

**Alphabet Programming Language v2.3.0**
**By Fraol Teshome | MIT License | github.com/fraol163/alphabet**
