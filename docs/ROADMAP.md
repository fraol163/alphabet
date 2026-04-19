# Alphabet Project Roadmap - v2.1.0

**Project Owner: Fraol Teshome**

## Completed (v2.0.0 - v2.1.0)

### Core Language
- [x] Lexer with 17 single-letter keywords
- [x] Recursive descent parser
- [x] Stack-based bytecode compiler
- [x] Virtual machine with 40 opcodes
- [x] Numeric type system (type IDs 0-14)
- [x] OOP: classes, methods, inheritance, interfaces, visibility
- [x] Exception handling (try/handle)
- [x] Pattern matching (match/case)
- [x] Break and continue (including nested loops)
- [x] C-style for loop
- [x] Import module system
- [x] FFI: call native C functions
- [x] String escape sequences
- [x] String concatenation with type coercion
- [x] Underscore identifiers

### Standard Library
- [x] math.abc (factorial, gcd, lcm, max, min, clamp, sign)
- [x] io.abc (print, println, read_file)

### Built-In Functions
- [x] Math: sqrt, abs, pow, floor, ceil, sin, cos
- [x] String: len, tostr, tonum, type

### Tooling
- [x] CLI with --repl, --lsp, --version, --help
- [x] REPL with persistent state
- [x] LSP server (JSON-RPC, completions, hover, diagnostics)
- [x] Compile-only mode with bytecode serialization
- [x] Error messages with line/column/source context
- [x] Debug mode with breakpoints

### Infrastructure
- [x] CMake build system with tests
- [x] GitHub Actions CI
- [x] GitHub Actions Release (Linux/macOS/Windows)
- [x] One-line install/update/uninstall script
- [x] Multilingual keywords (English, Amharic, Spanish, French, German)

---

## Future (v2.2+)

### Language Features
- [ ] Lambda functions
- [ ] Generics
- [ ] String interpolation
- [ ] Range expressions

### Standard Library
- [ ] Collections (sort, filter, map, reduce)
- [ ] File I/O (write, append, exists)
- [ ] JSON parsing
- [ ] HTTP client

### Tooling
- [ ] VS Code extension (syntax highlighting)
- [ ] Package manager
- [ ] Documentation generator
- [ ] Profiler

### Performance
- [ ] Bytecode specialization (int vs float opcodes)
- [ ] Inline caching for method lookups
- [ ] JIT compilation (long-term)

---
*Status: v2.1.0 Released. Core language complete. Focus on ecosystem.*
