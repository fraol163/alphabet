# Alphabet Language Reference

**Complete reference for Alphabet v2.0**

---

## Keywords (17 Letters)

| Letter | Keyword | Example |
|--------|---------|---------|
| `a` | Abstract | `a c Interface { }` |
| `b` | Break | `b` |
| `c` | Class | `c MyClass { }` |
| `e` | Else | `i (x) { } e { }` |
| `h` | Handle | `h (5 e) { }` |
| `i` | If | `i (x > 0) { }` |
| `j` | Interface | `j J { }` |
| `k` | Continue | `k` |
| `l` | Loop | `l (x > 0) { }` |
| `m` | Method | `v m 1 f() { }` |
| `n` | New | `15 obj = n MyClass()` |
| `p` | Private | `p 1 x = 10` |
| `r` | Return | `r x + y` |
| `s` | Static | `s 1 x = 5` |
| `t` | Try | `t { }` |
| `v` | Public | `v m 1 f() { }` |
| `z` | System | `z.o("Hello")` |

---

## Type IDs

| ID | Type | Example |
|----|------|---------|
| 1-4 | int (8/16/32/64) | `1 x = 100` |
| 5 | int (generic) | `5 x = 42` |
| 6-8 | float | `6 pi = 3.14` |
| 11 | bool | `11 ok = (1 == 1)` |
| 12 | string | `12 s = "Hello"` |
| 13 | list | `13 arr = [1, 2, 3]` |
| 14 | map | `14 m = {"id": 1}` |
| 15+ | custom class | `15 obj = n MyClass()` |

---

## Operators

```
Arithmetic:  +  -  *  /  %
Comparison:  == != >  <  >= <=
Logical:     && || !
Assignment:  =
```

---

## System Functions

| Function | Description |
|----------|-------------|
| `z.o(x)` | Output/print |
| `z.i()` | Input/read |
| `z.f(p)` | Read file |
| `z.t()` | Throw error |

---

**For more:** [Getting Started](GETTING_STARTED.md) · [Tutorial](TUTORIAL.md)
