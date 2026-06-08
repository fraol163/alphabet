# Changelog

## v2.3.5 (2026-06-07)

### Fixed
- Continue eating next statement as label (parser bug)
- Abstract class parsing (`a c ClassName` syntax)
- Static methods returning null (VM dispatching to wrong map)
- Constructor not being called (VM looked for `init`, not class name)
- `this.field` type inference (missing Get handler in compiler)
- For-each + continue infinite loop (increment after body)
- Labeled break/continue not working (labels ignored in compiler)
- Nested for-each variable name conflict (unique `__fe`/`__i` per level)
- WASM build (O1 optimization, fixed exported functions)
- STORE_FIELD stack underflow
- NullSafeGet type inference
- PUSH_CONST_POOL missing VM handler
- run_field_init() only handled PUSH_CONST, not PUSH_CONST_POOL

### Added
- `super()` call support (LOAD_SUPER opcode, parent class dispatch)
- `alphabet pkg` subcommand (install, list, search packages)
- Standard library modules (math, string_utils, json, collections, testing)
- Formal language specification (docs/SPEC.md, 1548 lines)
- Constant folding optimization pass (3+4 → 7 at compile time)
- Improved error messages with keyword conflict hints (26 keywords)
- REPL Trace Mode — 4-phase visual trace (Tokenizing, Parsing, Compiling, Executing)
- REPL Trace Slow Mode — watch each step happen gradually with delays
- REPL trace commands (trace on/off/slow/fast)
- Closures and lambdas: `m(type param) { body }`
- z.map(), z.filter(), z.reduce() with lambda support
- Ternary operator: `cond ? a : b`
- Voice input — Vosk for en/es/fr/de, Whisper for am
- Browser playground — WebAssembly VM, dark theme, 7 examples
- Threading — z.thread(), z.join(), z.join_all(), z.lock()
- Embedding API — alphabet_embed.h/cpp, Alphabet class
- Project management — alphabet init, alphabet info, alphabet.toml
- `alphabet doc` command — 82 builtin docs with signatures and descriptions
- `alphabet bench` command — 5 VM benchmarks
- Colored error output — ANSI red for errors, yellow for warnings
- CI/CD pipeline — build matrix, format check, ASan, coverage
- CONTRIBUTING.md — developer guide
- Dockerfile — multi-stage build
- Static analysis — .clang-format, .clang-tidy
- BNF grammar spec — docs/GRAMMAR.md
- CMake presets — default, debug, asan, coverage
- Localized stdlib — stdlib/{es,fr,de,am}/math.abc
- z.map(), z.filter(), z.reduce() — functional programming
- Closures — lambdas capture global variables
- Ternary operator
- Error recovery suggestions — parser hints
- Voice-to-text input — Vosk + Whisper
- NL-to-Code converter
- Threading support — z.thread(), z.join(), z.lock()
- 90 built-in functions
- Garbage collection — mark-and-sweep
- Bytecode serialization — save/load .abc files
- Bytecode optimizer — constant folding, NOP elimination
- Profiler — `--profile` flag
- VS Code extension — syntax highlighting + snippets
- Shell completions — bash, zsh, fish
- Man page
- 38 example programs
- 21 stdlib modules
- 257 test files

---

## v2.3.4 (2026-05-28)
### Added
- Visibility enforcement: private fields/methods enforced at runtime
- Do-while loop: `l { body } (condition)`
- Named type keywords: `int`, `str`, `bool`, `list`, `map`, `float`
- VM refactored: 76 handlers (one per opcode)
- For-each loop: `l (item : list) { body }`
- Range expressions: `1..10` → `[1, 2, ..., 9]`
- Null-safe operator: `obj?.field`
- Operator overloading: `__add`, `__sub`, etc.
- Abstract class enforcement
- Default parameters: `m 5 add(5 a, 5 b = 10) { r a + b }`
- Labeled break/continue: `outer: l (...) { b outer }`
- Bytecode version header (VERSION=1)
- z.sleep(), z.http_get(), z.http_post(), z.timestamp(), z.env()
- z.json_parse(), z.json_stringify() — full recursive descent JSON parser
- Stack overflow protection (MAX_CALL_DEPTH=1000)
- Instruction count limit (MAX_INSTRUCTIONS=10M)
- Memory limits (MAX_GLOBALS=10K, MAX_OBJECTS=100K)
- String indexing: `s[0]`, `s[-1]`
- String iteration: `l (c : "hello") { z.o(c) }`
- Tuples: `(1, "hello", true)` syntax
- Sets: `z.set()`, `z.add()`, `z.has()`, `z.set_size()`
- Destructuring: `[a, b] = [10, 20]` (works in any position)
- Stack traces on runtime errors
- `ei` keyword (else-if shorthand) — `e i` works, `ei` attempted
- Strict mode: `#alphabet<en strict>` enables unused variable warnings
- Warning system: unused variable detection in strict mode
- Profiler: `--profile` flag shows opcode execution counts
- Watch mode: `alphabet watch file.abc` re-runs on file change
- File:line in error messages
- Stdlib auto-discovery (tries stdlib/ relative to cwd and binary)
- VS Code syntax highlighting extension
- Shell completions (bash, zsh, fish)
- Man page (alphabet.1)
- Editor snippets (12 snippets)
- Interactive learning tour (10 lessons)
- i18n error message reference (en/am/es/fr/de)
- Code formatter: `alphabet fmt [-w] file.abc`
- Linter: `alphabet lint file.abc`
- Error codes: E001-E299 with descriptions
- REPL: q, quit, exit, help, clear, reset, vars, keywords, history, !!
- CLI: --version, --repl, --lsp, --debug, --sandbox, --dump-bytecode, --compile, --output, update

### Fixed
- STORE_FIELD stack underflow (pushes val back after storing)
- NullSafeGet type inference
- z.tonum(bool) returning 0
- Parser segfault on `a[0]` (abstract keyword conflict)
- Label support in for-loops (ForStmt.label field)
- Ternary operator in parser
- Range expressions (1..10 desugars to z.range())
- Destructuring after other statements (lookahead in declaration() + call())

## v2.3.3 (2026-05-20)
### Added
- REPL with history, multi-line, language switching
- LSP server for VS Code
- Debugger with breakpoints, step, locals, stack trace
- FFI — call native C functions from Alphabet code
- Sandbox mode
- Pattern matching (match/case)
- Exception handling (try/handle)
- f-string interpolation
- 40+ built-in functions
- Standard library (math, io, string, list)
