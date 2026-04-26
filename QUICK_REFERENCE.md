# Alphabet Language Quick Reference Card

**Print this page for quick access!**

---

## All 19 Keywords

| Letter | Keyword | Example | Description |
|--------|---------|---------|-------------|
| `a` | Abstract | `a c Interface { }` | Abstract class |
| `b` | Break | `b` | Exit loop |
| `c` | Class | `c MyClass { }` | Define class |
| `e` | Else | `i (x) { } e { }` | Else branch |
| `h` | Handle | `h (5 e) { }` | Catch exception |
| `i` | If | `i (x > 0) { }` | Conditional |
| `j` | Interface | `j J { }` | Interface |
| `k` | Continue | `k` | Next iteration |
| `l` | Loop | `l (x > 0) { }` | While loop |
| `m` | Method | `v m 1 f() { }` | Function |
| `n` | New | `n MyClass()` | Instantiate |
| `p` | Private | `p 1 x = 10` | Private access |
| `r` | Return | `r x + y` | Return value |
| `s` | Static | `s 1 x = 5` | Static member |
| `t` | Try | `t { }` | Try block |
| `v` | Public | `v m 1 f() { }` | Public access |
| `z` | System | `z.o("Hello")` | System functions |

---

## Type IDs

### Integers (1-5)
```
5 = int     (generic integer)
```

### Floats (6-10)
```
6 = f32     (32-bit float)
7 = f64     (64-bit float)
8 = float   (generic float)
```

### Other (11-14)
```
11 = bool   (true/false)
12 = str    (string)
13 = list   (array)
14 = map    (dictionary)
```

### Custom (15+)
```
15+ = First custom class, second class, etc.
```

---

## Operators

```
Arithmetic:  +  -  *  /  %
Comparison:  ==  !=  >  <  >=  <=
Logical:     &&  ||  !
Access:      .  @  ^
```

---

## System Functions (z.*)

### I/O
```
z.o(x)          # Print/output
z.i()           # Read input (returns string or number)
z.f(path)       # Read file contents
z.fw(path, data)# Write to file
```

### Math
```
z.sqrt(x)       # Square root
z.abs(x)        # Absolute value
z.pow(a, b)     # a^b
z.floor(x)      # Floor
z.ceil(x)       # Ceiling
z.sin(x)        # Sine
z.cos(x)        # Cosine
z.tan(x)        # Tangent
z.round(x)      # Round to nearest integer
z.min(a, b)     # Minimum of two values
z.max(a, b)     # Maximum of two values
z.log(x)        # Natural logarithm
z.log10(x)      # Base-10 logarithm
```

### Type & Conversion
```
z.type(x)       # "number", "string", "list", "map", "object", "null", etc.
z.len(x)        # Length of string, list, or map
z.tostr(x)      # Convert to string
z.tonum(x)      # Convert to number
```

### String Operations
```
z.upper(s)          # UPPERCASE
z.lower(s)          # lowercase
z.trim(s)           # Remove whitespace
z.split(s, delim)   # Split into list
z.join(list, sep)   # Join list to string
z.replace(s, old, new)  # Replace all
z.substr(s, start, len) # Substring
z.chr(code)         # Char from code
z.ord(char)         # Code from char
z.starts_with(s, p) # Check prefix (1/0)
z.ends_with(s, s)   # Check suffix (1/0)
z.contains(s, sub)  # Check substring (1/0)
z.reverse(s)        # Reverse string
```

### List Operations
```
z.append(list, val)     # Add to end (mutates)
z.pop_back(list)        # Remove last, return it
z.contains(list, val)   # Check membership (1/0)
z.reverse(list)         # New reversed list
z.insert(list, i, val)  # Insert val at index i (mutates)
z.remove(list, i)       # Remove element at index i (mutates)
```

### Range Generation
```
z.range(stop)           # [0, 1, ..., stop-1]
z.range(start, stop)    # [start, ..., stop-1]
z.range(start, stop, step)  # With step
```

### Map Operations
```
z.keys(map)             # List of keys
z.values(map)           # List of values
```

### Errors
```
z.t()                   # Throw error
z.t("msg")              # Throw with message
```

---

## Standard Library

```
x "stdlib/math.abc"     # factorial, gcd, lcm, max, min, clamp, sign
x "stdlib/io.abc"       # print, println, read_file
x "stdlib/string.abc"   # contains, split, join, replace, trim, upper, lower, ...
x "stdlib/list.abc"     # length, push, pop, contains, reverse, range, keys, ...
```

---

## Syntax Patterns

### Hello World
```alphabet
#alphabet<en>
12 msg = "Hello, World!"
z.o(msg)
```

### Variables
```alphabet
5 x = 10              # int
6 pi = 3.14           # float
12 name = "Alphabet"  # string
13 nums = [1, 2, 3]   # list
14 cfg = {"k": 100}   # map
```

### String Interpolation (f-strings)
```alphabet
5 age = 25
12 name = "Alice"
z.o(f"Hello, {name}!")       # Hello, Alice!
z.o(f"You are {age} old.")   # You are 25 old.
z.o(f"2+3 = {2 + 3}")       # 2+3 = 5
```

### Negative Indexing
```alphabet
13 nums = [10, 20, 30]
z.o(nums[-1])  # 30 (last element)
z.o(nums[-2])  # 20
```

### For-Style Loop
```alphabet
l (5 j = 0 : j < 5 : j = j + 1) {
  z.o(j)
}
```

### Range Loop
```alphabet
13 r = z.range(5)
l (5 i = 0 : i < z.len(r) : i = i + 1) {
  z.o(r[i])
}
```

### Class
```alphabet
c Calculator {
  v m 5 add(5 a, 5 b) {
    r a + b
  }
}
15 calc = n Calculator()
z.o(calc.add(15, 25))  # 40
```

### Pattern Matching
```alphabet
q (x) {
  1: z.o("one")
  2: z.o("two")
  e: z.o("other")
}
```

---

## Command Line

```bash
alphabet program.abc              # Run program
alphabet --repl                   # Interactive REPL
alphabet --lsp                    # LSP server
alphabet --debug program.abc      # Debug with breakpoints
alphabet --dump-bytecode prog.abc # Inspect bytecode
alphabet --sandbox program.abc    # Sandboxed execution
alphabet -c -o out.bin prog.abc   # Compile to bytecode
alphabet update                   # Self-update
alphabet --version                # Show version
alphabet --help                   # Show help
```

---

## Debugger Commands (in --debug mode)

```
continue (c)      Resume execution
step (s)          Step to next line
locals (l)        Show local variables
globals (g)       Show global variables
stack (bt)        Show call stack trace
print (p)         Show stack contents
add_break N (b)   Set breakpoint at line N
del_break N (db)  Remove breakpoint
breakpoints (bl)  List breakpoints
help (?)          Show help
```

---

## Quick Tips

Start every file with `#alphabet<en>`
Use type 5 for integers, 12 for strings, 13 for lists
Negative indexing: list[-1] gets last element
String concat: "hello" + 42 = "hello42"
String interpolation: f"Hello {name}" embeds variables in strings
All numbers are stored as double internally
Type mismatches in arithmetic throw RuntimeError with clear messages

---

**Version:** 2.3.3 | **Implementation:** Native C++17 | **License:** MIT
**GitHub:** https://github.com/fraol163/alphabet
