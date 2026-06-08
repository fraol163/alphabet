# Alphabet Language Benchmarks

**Real performance measurements with verified data.**

**Last Updated:** May 20, 2026

---

## Summary

**Note:** Results depend on hardware. Run benchmarks on your machine for accurate numbers.

Alphabet v2.3.5 uses a stack-based bytecode interpreter (no JIT). Performance characteristics:

- **Recursive workloads:** ~1.7x slower than Python (fibonacci)
- **Tight loops:** ~1.1x comparable to Python (100K iteration loop)
- **Startup:** Fast (native binary, no runtime boot)

Integer-typed operations (v2.3.5 real int64_t support) significantly improved performance over the previous double-only implementation.

---

## Test: Recursive Fibonacci (n=20)

| Language | Time | Relative |
|----------|------|----------|
| **Python 3** | 0.108s | 1.0x |
| **Alphabet** | 0.184s | 1.7x |

```alphabet
#alphabet<en>
m 5 fib(5 num) {
  i (num <= 1) { r num }
  r fib(num - 1) + fib(num - 2)
}
z.o(fib(20))
```

## Test: Tight Loop (100K iterations)

| Language | Time | Relative |
|----------|------|----------|
| **Python 3** | 0.219s | 1.0x |
| **Alphabet** | 0.244s | 1.1x |

```alphabet
#alphabet<en>
5 sum = 0
5 ii = 0
l (ii < 100000) {
  sum = sum + ii
  ii = ii + 1
}
z.o(sum)
```

---

## Performance Notes

The Alphabet VM is a straightforward stack interpreter:
- Each instruction dispatched via switch/case
- No bytecode optimization or JIT
- Dynamic type checks on every operation
- Real integer (int64_t) support since v2.3.5 avoids float overhead

Python's CPython interpreter is also a stack VM, but with decades of optimization (specialized opcodes, inline caches, etc.).

## Optimization Roadmap

1. **Bytecode specialization** - separate opcodes for int vs float ops
2. **Inline caching** - cache field/method lookups
3. **Register-based VM** - reduce stack manipulation overhead
4. **JIT compilation** - compile hot paths to native code (long-term)
