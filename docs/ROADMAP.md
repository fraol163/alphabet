# Alphabet Project Roadmap - v2.3.1

**Project Owner: Fraol Teshome**

## Completed (v2.0.0 - v2.3.1)

### Core Language
- [x] Lexer with 19 single-letter keywords
- [x] Recursive descent parser
- [x] Stack-based bytecode compiler
- [x] Virtual machine with 40+ opcodes
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
- [x] Python-style negative list indexing (list[-1])
- [x] Dynamic stack (vector-based, grows on demand)
- [x] Const variable declarations
- [x] --dump-bytecode flag for debugging

### Standard Library
- [x] math.abc (factorial, gcd, lcm, max, min, clamp, sign)
- [x] io.abc (print, println, read_file)
- [x] string.abc (contains, starts_with, ends_with, split, join, replace, trim, upper, lower, substr, reverse, length, chr, ord)
- [x] list.abc (length, push, pop, contains, reverse, first, last, range, range_from, range_step, keys, values)

### Built-In Functions
- [x] Math: sqrt, abs, pow, floor, ceil, sin, cos
- [x] String: len, tostr, tonum, type, split, join, replace, trim, upper, lower, substr, chr, ord, starts_with, ends_with
- [x] List: append, pop_back, contains, reverse, range, keys, values

### Tooling
- [x] CLI with --repl, --lsp, --version, --help, --dump-bytecode, --sandbox, --debug
- [x] REPL with persistent state and error recovery
- [x] LSP server (JSON-RPC, completions, hover, diagnostics)
- [x] Compile-only mode with bytecode serialization
- [x] Error messages with line/column/source context
- [x] Debug mode with breakpoints and step
- [x] --dump-bytecode flag for inspecting compiled output

### Infrastructure
- [x] CMake build system with unit, golden, and language tests
- [x] GitHub Actions CI with clang-format enforcement
- [x] GitHub Actions Release (Linux/macOS/Windows)
- [x] One-line install/update/uninstall script
- [x] Multilingual keywords (English, Amharic, Spanish, French, German)
- [x] Version managed from single VERSION file
- [x] Dynamic VM stack (no more fixed 64KB allocation)

---

## v2.4.0 - Ecosystem

### Language Expansion
- [ ] Portuguese keywords (pt)
- [ ] Chinese keywords (zh) - simplified
- [ ] Hindi keywords (hi)
- [ ] Arabic keywords (ar) - RTL support
- [ ] Japanese keywords (ja)
- [ ] Korean keywords (ko)
- [ ] Community translation framework (contributor-friendly keyword mapping system)

### Package Manager (alphabet-pkg)
- [ ] Package manifest format (alphabet.toml)
- [ ] Local package cache (~/.alphabet/packages/)
- [ ] Install from GitHub repos: `alphabet install github:user/repo`
- [ ] Install from URL: `alphabet install https://example.com/lib.abc`
- [ ] Dependency resolution and version pinning
- [ ] `alphabet init` - scaffold new project
- [ ] `alphabet build` - build project with dependencies
- [ ] `alphabet test` - run project test suite
- [ ] Central registry (alphabet-pkg.dev) - long-term goal
- [ ] Homebrew tap for macOS users
- [ ] apt repository for Debian/Ubuntu users

---

## Future (v2.5+)

### Language Features
- [ ] Lambda functions
- [ ] Generics
- [ ] String interpolation (`f"Hello {name}"`)
- [ ] Range expressions in for loops
- [ ] Destructuring assignment
- [ ] Enum types

### Standard Library
- [ ] Collections (sort, filter, map, reduce as stdlib functions)
- [ ] File I/O (write, append, exists, delete)
- [ ] JSON parsing
- [ ] HTTP client
- [ ] Date/time handling

### Tooling
- [ ] VS Code extension (syntax highlighting, snippets)
- [ ] Documentation generator from source comments
- [ ] Profiler and benchmark tools
- [ ] Package registry website

### Performance
- [ ] Bytecode specialization (int vs float opcodes)
- [ ] Inline caching for method lookups
- [ ] Constant folding optimization
- [ ] JIT compilation (long-term)

---

*Status: v2.3.1 Released. Core language and standard library complete. Focus on ecosystem and internationalization.*
