# Alphabet Language Benchmarks

**Real performance measurements with verified data.**

**Last Updated:** April 19, 2026

---

## Summary

Alphabet V2 uses a stack-based bytecode interpreter (no JIT). Performance characteristics:

- **Recursive workloads:** ~11x slower than Python (fibonacci)
- **Tight loops:** ~32x slower than Python (100K iteration loop)
- **Startup:** Fast (native binary, no runtime boot)

These numbers are for the current interpreter implementation. Future JIT compilation could significantly improve performance.

---

## Test: Recursive Fibonacci (n=20)

| Language | Time | Relative |
|----------|------|----------|
| **Alphabet** | 2.81s | 11.2x |
| **Python 3** | 0.25s | 1.0x |

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
| **Alphabet** | 6.37s | 31.9x |
| **Python 3** | 0.20s | 1.0x |

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

## Why Slower Than Python?

The Alphabet VM is a straightforward stack interpreter:
- Each instruction dispatched via switch/case
- No bytecode optimization or JIT
- Dynamic type checks on every operation
- Function calls allocate new CallFrame objects

Python's CPython interpreter is also a stack VM, but with decades of optimization (specialized opcodes, inline caches, etc.).

## Optimization Roadmap

1. **Bytecode specialization** - separate opcodes for int vs float ops
2. **Inline caching** - cache field/method lookups
3. **Register-based VM** - reduce stack manipulation overhead
4. **JIT compilation** - compile hot paths to native code (long-term)
