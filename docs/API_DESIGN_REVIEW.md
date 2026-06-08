# Alphabet Language — API Design Review

## Builtin Function Naming

### Current Pattern
```
z.o()        — output (print)
z.i()        — input
z.f()        — file read
z.fw()       — file write
z.fa()       — file append
z.len()      — length
z.type()     — type name
z.tostr()    — to string
z.tonum()    — to number
```

### Consistency Analysis

**Consistent:**
- z.o/z.i — I/O pair ✅
- z.f/z.fw/z.fa — File operations ✅
- z.tostr/z.tonum — Type conversion ✅
- z.upper/z.lower — String case ✅
- z.keys/z.values — Map operations ✅

**Inconsistent:**
- z.len() — works on strings, lists, maps (good)
- z.type() — returns string, not type ID
- z.sum()/z.avg() — only work on lists of numbers
- z.find() — works on strings, not lists

### Recommendations

1. **Add z.find() for lists** — search list elements
2. **Add z.contains() alias** — z.has() for consistency
3. **Add z.flat() alias** — z.flatten() already exists
4. **Add z.trim() for lists** — remove nulls from ends

## CLI Command Naming

### Current Pattern
```
alphabet run      — execute file
alphabet test     — run tests
alphabet lint     — check code
alphabet bench    — benchmarks
alphabet doc      — documentation
alphabet init     — create project
alphabet info     — project info
alphabet examples — list examples
alphabet tour     — interactive tour
alphabet update   — check updates
```

### Consistency Analysis

**Consistent:**
- All lowercase ✅
- Single-word commands ✅
- Verb-based (run, test, lint, bench) ✅

**Inconsistent:**
- `alphabet voice-tutorial` — hyphenated (should be `alphabet voice-tutorial` or `alphabet vtutorial`)
- `alphabet --repl` — flag, not subcommand

### Recommendations

1. **Keep `alphabet voice-tutorial`** — descriptive enough
2. **Add `alphabet repl`** — as alias for `--repl`
3. **Add `alphabet fmt`** — code formatter (future)

## Error Message Quality

### Current Errors
```
Parse Error: Expect ')' after expression.
  Hint: check for missing closing parenthesis ✅

Runtime Error: Stack underflow ❌ (not helpful)

Compile Error: Type mismatch ❌ (needs more context)
```

### Recommendations

1. **Add context to runtime errors** — show variable names
2. **Add "did you mean?"** — spelling suggestions
3. **Add error codes** — for documentation lookup
4. **Add line numbers to runtime errors** — show where error occurred
