# Alphabet Language Quick Reference Card

**Print this page for quick access!**

---

## All 17 Keywords

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

**Unused (Reserved):** d, f, g, q, u, w, x, y

---

## Type IDs

### Integers (1-5)

```
1 = i8      (8-bit)
2 = i16     (16-bit)
3 = i32     (32-bit)
4 = i64     (64-bit)
5 = int     (generic)
```

### Floats (6-10)

```
6 = f32     (32-bit float)
7 = f64     (64-bit float)
8 = float   (generic float)
9 = dec     (decimal)
10 = cpx    (complex)
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

### Arithmetic
```
+  -  *  /  %
```

### Comparison
```
==  !=  >  <  >=  <=
```

### Logical
```
&&  ||  !
```

### Assignment
```
=  +=  -=  *=  /=
```

### Access
```
.  @  ^
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
5 x = 10          # int
6 pi = 3.14       # float
12 name = "Alphabet"  # string
11 ok = (1 == 1)  # bool
```

### If Statement
```alphabet
i (x > 0) {
  z.o("positive")
} e {
  z.o("non-positive")
}
```

### Loop
```alphabet
5 i = 0
l (i < 10) {
  z.o(i)
  5 i = i + 1
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
5 result = calc.add(15, 25)
```

### Exception Handling
```alphabet
t {
  # risky code
  z.t()
} h (12 e) {
  z.o("Error: " + e)
}
```

---

## System Functions

### Input/Output
```
z.o(x)        # Print/output
z.i()         # Read input
```

### Files
```
z.f(path)     # Read file
z.w(path,d)   # Write file
```

### Errors
```
z.t()         # Throw error
```

### External (FFI)
```
z.load_lib(p) # Load library
lib.call(f)   # Call function
```

---

## Common Patterns

### Swap Variables
```alphabet
5 temp = a
5 a = b
5 b = temp
```

### Increment
```alphabet
5 x = x + 1
```

### Check Even
```alphabet
i (x % 2 == 0) {
  z.o("even")
}
```

### Array Length
```alphabet
13 arr = [1, 2, 3]
5 len = arr.length()
```

---

## Error Messages

| Error | Meaning | Fix |
|-------|---------|-----|
| `MissingLanguageHeader` | No `#alphabet<lang>` | Add magic header |
| `TypeMismatch` | Wrong type | Check type IDs |
| `UndefinedVariable` | Variable not declared | Declare first |
| `StackOverflow` | Too much recursion | Reduce depth |

---

## Command Line

```bash
# Run program
alphabet program.abc

# REPL mode
alphabet --repl

# LSP server
alphabet --lsp

# Show version
alphabet --version

# Show help
alphabet --help
```

---

## Build from Source

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

## Quick Tips

✅ **Do:**
- Start every file with `#alphabet<en>`
- Use ASCII for class/method names
- Use appropriate type IDs
- Keep recursion under 1000 levels

❌ **Don't:**
- Forget the magic header
- Use semicolons (not needed!)
- Mix types without conversion
- Use Unicode identifiers (not supported)

---

## File Extension

All Alphabet files use: **`.abc`**

---

## Magic Header

Must be first line: **`#alphabet<lang>`**

Valid lang codes: `en`, `es`, `fr`, `de`, etc.

---

## Memory Layout

```
Stack: 65536 slots (~3MB fixed)
Heap: Dynamic (strings, lists, maps, objects)
```

---

## Performance Tips

1. Use smaller type IDs when possible
2. Minimize object creation in loops
3. Use local variables
4. Prefer iteration over recursion

---

## Learning Path

1. **GETTING_STARTED.md** - 10 minutes
2. **TUTORIAL.md** - 2 hours (10 lessons)
3. **REFERENCE.md** - Complete syntax
4. **ADVANCED.md** - FFI, LSP, architecture
5. **BENCHMARKS.md** - Performance data

---

## Resources

- **GitHub:** https://github.com/fraol163/alphabet
- **Issues:** https://github.com/fraol163/alphabet/issues
- **Discussions:** https://github.com/fraol163/alphabet/discussions
- **Docs:** See `docs/` folder

---

## Contact

**Fraol Teshome**  
Email: fraolteshome444@gmail.com

---

**License:** MIT  
**Version:** 2.0.0  
**Implementation:** Native C++17

---

**Print date:** _______________  
**Your name:** _______________

---

*Cut along the line to fit in your wallet!*
