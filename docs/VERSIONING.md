# Alphabet Language Versioning Policy

## Semver Convention

Alphabet follows [Semantic Versioning](https://semver.org/):

```
MAJOR.MINOR.PATCH
  │     │     └── Bug fixes, no breaking changes
  │     └──────── New features, backward compatible
  └────────────── Breaking changes
```

## Current Version

**v2.3.5** — Released May 31, 2026

## Version History

| Version | Date | Highlights |
|---------|------|------------|
| v2.3.5 | 2026-05-31 | Closures, z.map/filter/reduce, ternary, error hints |
| v2.3.4 | 2026-05-28 | Visibility enforcement, do-while, named types, lambdas |
| v2.3.3 | 2026-05-15 | REPL redesign, brace tracking, history |
| v2.3.2 | 2026-05-01 | F-string expressions, match/case |
| v2.3.1 | 2026-04-15 | Exception handling, try/handle |
| v2.3.0 | 2026-04-01 | OOP: classes, inheritance, interfaces |
| v2.2.0 | 2026-03-15 | Standard library, modules |
| v2.1.0 | 2026-03-01 | Type system, type checking |
| v2.0.0 | 2026-02-15 | Bytecode VM, 5 languages |

## Stability Guarantees

### What's Stable (won't break without major version bump)
- All 19 keywords
- `z.` builtin function signatures
- `#alphabet<lang>` header format
- Bytecode format (ALPH header)
- REPL commands
- CLI flags

### What May Change (minor version)
- New opcodes added
- New builtins added
- New keywords added (single-letter + full word aliases)
- Stdlib modules expanded
- Error messages improved

### What's Experimental (may change anytime)
- `--profile` flag output format
- `alphabet bench` benchmark numbers
- Embedding API (`alphabet_embed.h`)
- Bytecode cache format (`.abcc`)

## Deprecation Policy

1. Feature marked deprecated in changelog
2. Warning emitted for 2 minor versions
3. Feature removed in next major version

## How to Check Version

```bash
alphabet --version
# Alphabet 2.3.5 (Native C++)
```
