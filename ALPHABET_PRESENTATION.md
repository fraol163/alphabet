# Alphabet Programming Language
## Presentation Guide

---

## Slide 1: Title

# ALPHABET
### The Fastest Way to Learn Programming

**17 Keywords. Infinite Possibilities.**

*Fraol Teshome | github.com/fraol163/alphabet*

---

## Slide 2: The Problem

### Why Do Students Quit Programming?

- Too many keywords to memorize (Python: 35, Java: 50+)
- Syntax errors distract from concepts
- Setup complexity (compilers, build systems, environments)
- Languages designed for professionals, not learners

**We need a language designed for learning, not production.**

---

## Slide 3: The Solution

### Alphabet: 17 Single-Letter Keywords

```
a = abstract      i = if          p = private
b = break         j = interface   r = return
c = class         k = continue    s = static
e = else          l = loop        t = try
h = handle        m = method      v = public
                  n = new         z = output
```

**Learn the entire language in 10 minutes.**

---

## Slide 4: Hello World

### Your First Program

```alphabet
#alphabet<en>
12 greeting = "Hello, World!"
z.o(greeting)
```

**That's it.** No main function, no imports, no semicolons.

Output: `Hello, World!`

---

## Slide 5: Variables and Types

### Type System by Numbers

```alphabet
5 age = 25              # integer
6 pi = 3.14             # float
12 name = "Alphabet"    # string
11 is_ready = 1         # boolean
13 items = [1, 2, 3]    # list
14 config = {"k": 100}  # map
```

Type IDs: `5`=int, `12`=string, `13`=list, `14`=map

**Intuitive once you learn the pattern.**

---

## Slide 6: Control Flow

### If/Else and Loops

```alphabet
5 score = 85
i (score >= 90) {
  z.o("A")
} i (score >= 80) {
  z.o("B")
} e {
  z.o("C")
}
```

```alphabet
5 i = 0
l (i < 5) {
  z.o(i)
  5 i = i + 1
}
```

---

## Slide 7: Functions

### Clean Function Syntax

```alphabet
m 5 factorial(5 num) {
  i (num <= 1) { r 1 }
  r num * factorial(num - 1)
}

z.o(factorial(5))  # 120
```

Syntax: `m <return_type> <name>(<params>) { body }`

---

## Slide 8: Object-Oriented Programming

### Classes with Inheritance

```alphabet
c Animal {
  v m 12 speak() { r "..." }
}

c Dog ^ Animal {
  v m 12 speak() { r "Woof!" }
}

15 pet = n Dog()
z.o(pet.speak())  # Woof!
```

`^` = extends | `v` = public | `p` = private | `s` = static

---

## Slide 9: Collections

### Lists, Maps, and Ranges

```alphabet
13 nums = [10, 20, 30]
z.o(nums[-1])           # 30 (negative indexing!)

14 user = {"name": "Fraol", "age": 25}
z.o(user["name"])       # Fraol

13 steps = z.range(5)   # [0, 1, 2, 3, 4]
```

---

## Slide 10: String Operations

### 20+ Built-In String Functions

```alphabet
12 s = "Hello, World!"
z.o(z.upper(s))              # HELLO, WORLD!
z.o(z.replace(s, "World", "Alphabet"))  # Hello, Alphabet!
13 parts = z.split(s, ",")   # ["Hello", " World!"]
z.o(z.contains(s, "World"))  # 1
```

Also: `trim`, `lower`, `substr`, `chr`, `ord`, `starts_with`, `ends_with`, `reverse`, `join`

---

## Slide 11: Pattern Matching

### Elegant Multi-Branch Logic

```alphabet
5 day = 3
q (day) {
  1: z.o("Monday")
  2: z.o("Tuesday")
  3: z.o("Wednesday")
  e: z.o("Other")
}
```

Output: `Wednesday`

---

## Slide 12: Exception Handling

### Safe Error Management

```alphabet
t {
  5 result = risky_operation()
} h (12 e) {
  z.o("Caught: " + e)
}
```

Throw custom errors:
```alphabet
z.t("Something went wrong")
```

---

## Slide 13: Multilingual Support

### Code in Your Native Language

**English:** `if (x > 0) { print("big") }`

**Amharic:** `ከሆነ (x > 0) { ውጤት.o("big") }`

**Spanish:** `si (x > 0) { imprimir.o("big") }`

**French:** `si (x > 0) { afficher.o("big") }`

**German:** `wenn (x > 0) { ausgeben.o("big") }`

**5 languages. Same language underneath.**

---

## Slide 14: Built-In Functions

### 30+ Functions Out of the Box

| Category | Functions |
|----------|-----------|
| Math | sqrt, abs, pow, floor, ceil, sin, cos |
| String | upper, lower, trim, split, join, replace, substr, chr, ord |
| List | append, pop_back, contains, reverse, range |
| Map | keys, values |
| Type | type, len, tostr, tonum |
| I/O | print, input, read_file |

All accessed via `z.function_name()`.

---

## Slide 15: Standard Library

### Four Ready-to-Use Modules

```
stdlib/math.abc     - factorial, gcd, lcm, max, min, clamp, sign
stdlib/io.abc       - print, println, read_file
stdlib/string.abc   - contains, split, join, replace, trim, upper, lower, ...
stdlib/list.abc     - length, push, pop, contains, reverse, range, keys, ...
```

```alphabet
x "stdlib/math.abc"
z.o(factorial(5))  # 120
```

---

## Slide 16: Developer Tools

### Complete Toolchain

| Tool | Command | Purpose |
|------|---------|---------|
| Run | `alphabet prog.abc` | Execute programs |
| REPL | `alphabet --repl` | Interactive mode |
| Debug | `alphabet --debug prog.abc` | Breakpoints, step, inspect |
| Bytecode | `alphabet --dump-bytecode prog.abc` | Inspect compiled output |
| LSP | `alphabet --lsp` | Editor integration |
| Sandbox | `alphabet --sandbox prog.abc` | Safe execution |

---

## Slide 17: Debugger

### Full-Featured Debugger

```bash
$ alphabet --debug myprogram.abc
{"event":"stopped","line":5,"reason":"breakpoint"}
> locals
{"x":"42","name":"Alphabet"}
> stack
{"frames":[{"index":0,"ip":12}],"depth":1}
> continue
```

Commands: continue, step, locals, globals, stack, print, breakpoints, help

---

## Slide 18: Architecture

### How It Works

```
Source (.abc)
    |
    v
  Lexer -----> Tokens
    |
    v
  Parser -----> AST (Abstract Syntax Tree)
    |
    v
 Compiler ----> Bytecode (43 opcodes)
    |
    v
   VM --------> Output
```

**Native C++17. No dependencies. Fast compilation.**

---

## Slide 19: Comparison

### Alphabet vs Other Languages

| Feature | Alphabet | Python | Scratch | C |
|---------|----------|--------|---------|---|
| Keywords | 17 | 35 | N/A | 32 |
| Hello World | 2 lines | 1 line | Blocks | 5 lines |
| Type System | Simple IDs | Dynamic | None | Manual |
| Multilingual | 5 languages | 1 | 70+ | 1 |
| Compiled | Bytecode | Interpreted | Interpreted | Native |
| Best For | Education | Scripts | Kids | Systems |

---

## Slide 20: Installation

### One Command Install

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

Works on Linux, macOS, Windows (WSL).

**Or build from source:**

```bash
git clone https://github.com/fraol163/alphabet.git
cd alphabet
mkdir build && cd build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
```

---

## Slide 21: Live Demo Script

### Demo 1: Hello World (30 seconds)

```alphabet
#alphabet<en>
12 msg = "Hello from Alphabet!"
z.o(msg)
```

### Demo 2: Loops and Functions (60 seconds)

```alphabet
#alphabet<en>
m 5 fibonacci(5 num) {
  i (num <= 1) { r num }
  r fibonacci(num - 1) + fibonacci(num - 2)
}

l (5 i = 0 : i < 10 : i = i + 1) {
  z.o(fibonacci(i))
}
```

### Demo 3: Classes (60 seconds)

```alphabet
#alphabet<en>
c Calculator {
  5 result = 0
  v m 5 add(5 a, 5 b) {
    5 result = a + b
    r result
  }
}

15 calc = n Calculator()
z.o(calc.add(10, 20))  # 30
```

### Demo 4: Multilingual (30 seconds)

```alphabet
#alphabet<am>
5 ቁ = 42
ውጤት.o(ቁ)
```

---

## Slide 22: Roadmap

### What's Coming

**v2.4 - Ecosystem:**
- More languages (Portuguese, Chinese, Hindi, Arabic)
- Package manager (alphabet-pkg)
- Homebrew and apt packages

**v2.5+ - Advanced:**
- Lambda functions
- String interpolation
- VS Code extension
- Package registry

---

## Slide 23: Get Involved

### Join the Community

- **GitHub:** github.com/fraol163/alphabet
- **Issues:** Report bugs, request features
- **Contributions:** Code, docs, translations
- **Email:** fraolteshome444@gmail.com

**Star the repo. Share with educators. Teach with Alphabet.**

---

## Slide 24: Thank You

# Thank You

### Alphabet Programming Language

**17 Keywords. Learn in 10 Minutes. Build Anything.**

*Fraol Teshome*
*Addis Ababa, Ethiopia*
*github.com/fraol163/alphabet*

---

## Speaker Notes

### Key Talking Points

1. **Problem/Solution:** Emphasize the pain of learning to code with complex syntax. Alphabet removes that barrier.

2. **17 Keywords:** Show that you can list ALL keywords on one slide. Compare to Python's 35 or Java's 50+.

3. **Multilingual:** This is unique. No other compiled language supports writing code in Amharic, Spanish, French, and German natively.

4. **Live Demo:** Always run the hello world live. The instant feedback is impressive.

5. **Architecture:** It's a real compiler pipeline (lexer -> parser -> compiler -> VM), not a toy interpreter. This is a real language.

6. **For Educators:** Position Alphabet as a teaching tool, not a replacement for Python/Java. Students learn concepts here, then graduate to production languages.

### Common Questions

**Q: Why single-letter keywords?**
A: Reduces cognitive load. Students focus on WHAT they want to do (loop, check, return) not HOW to spell it.

**Q: Is it fast enough?**
A: It's a bytecode VM, not an interpreter. Fast enough for education and prototyping.

**Q: Can I build real programs?**
A: Yes - classes, exceptions, file I/O, FFI to C libraries. It's a real language, just simpler syntax.

**Q: Why C++17?**
A: No runtime dependencies. Single binary. Cross-platform. Fast compilation.
