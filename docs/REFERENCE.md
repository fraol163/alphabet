# Alphabet Language Reference

**Complete reference for Alphabet v2.3.5**

---

## Keywords (19 Letters)

| Letter | Keyword | Example |
|--------|---------|---------|
| `a` | Abstract | `a c Interface { }` |
| `b` | Break | `b` |
| `c` | Class | `c MyClass { }` |
| `e` | Else | `i (x) { } e { }` |
| `h` | Handle | `h (5 e) { }` |
| `i` | If | `i (x > 0) { }` |
|| `j` | Interface | `j J { }` |
|| `k` | Continue | `k` |
|| `l` | Loop | `l (x > 0) { }` |
|| `m` | Method | `v m 1 f() { }` |
|| `n` | New | `15 obj = n MyClass()` |
|| `p` | Private | `p 1 x = 10` |
|| `q` | Match | `q (x) { 1: ... }` |
|| `r` | Return | `r x + y` |
|| `s` | Static | `s 1 x = 5` |
|| `t` | Try | `t { }` |
|| `v` | Public | `v m 1 f() { }` |
|| `x` | Import | `x "module.abc"` |
|| `z` | System | `z.o("Hello")` |

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

### I/O
| Function | Description |
|----------|-------------|
| `z.o(x)` | Output/print |
| `z.i()` | Input/read |
| `z.f(p)` | Read file |
| `z.fw(p, data)` | Write file |
| `z.t()` | Throw error |

### Math
| Function | Description |
|----------|-------------|
| `z.sqrt(x)` | Square root |
| `z.abs(x)` | Absolute value |
| `z.pow(a, b)` | Power |
| `z.floor(x)` | Floor |
| `z.ceil(x)` | Ceiling |
| `z.round(x)` | Round |
| `z.sin(x)` | Sine |
| `z.cos(x)` | Cosine |
| `z.tan(x)` | Tangent |
| `z.log(x)` | Natural log |
| `z.log10(x)` | Base-10 log |
| `z.min(a, b)` | Minimum |
| `z.max(a, b)` | Maximum |

### String
| Function | Description |
|----------|-------------|
| `z.len(x)` | Length (string/list/map) |
| `z.type(x)` | Type name |
| `z.upper(s)` | Uppercase |
| `z.lower(s)` | Lowercase |
| `z.split(s, d)` | Split string |
| `z.join(l, s)` | Join list |
| `z.replace(s, old, new)` | Replace substring |
| `z.trim(s)` | Trim whitespace |
| `z.substr(s, start, len)` | Substring |
| `z.chr(n)` | Char from code |
| `z.ord(c)` | Code from char |
| `z.starts_with(s, prefix)` | Check prefix |
| `z.ends_with(s, suffix)` | Check suffix |
| `z.tostr(x)` | To string |
| `z.tonum(x)` | To number |

### Collection
| Function | Description |
|----------|-------------|
| `z.sort(l)` | Sort list |
| `z.reverse(l)` | Reverse list |
| `z.append(l, val)` | Append to list |
| `z.pop_back(l)` | Remove last element |
| `z.insert(l, i, val)` | Insert at index |
| `z.remove(l, i)` | Remove at index |
| `z.contains(l, val)` | Check if contains |
| `z.range(start, end, step)` | Generate range |
| `z.keys(m)` | Map keys |
| `z.values(m)` | Map values |

---

**For more:** [Getting Started](GETTING_STARTED.md) · [Tutorial](TUTORIAL.md)
