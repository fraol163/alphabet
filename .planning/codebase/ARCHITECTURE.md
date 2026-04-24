# Architecture

**Analysis Date:** 2025-02-13

## Pattern Overview

**Overall:** Compiled Virtual Machine Architecture

The Alphabet language follows a classic compiler-VM pipeline but with a unique localization layer at the lexing stage. It supports writing code in multiple natural languages (English, Amharic, Spanish, French, German) by mapping localized keywords to internal single-letter primitives.

**Key Characteristics:**
- **Multilingual Lexing:** First-pass translation of source keywords based on a magic header `#alphabet<lang>`.
- **Bytecode-Based Execution:** Source is compiled into a stack-based bytecode format executed by a custom Virtual Machine.
- **Strong Typing:** Uses a numeric type ID system (e.g., 5 for INT, 12 for STR) for type safety at compile and runtime.
- **Single-Letter Core:** The core language syntax uses single-character keywords (i=if, e=else, r=return) to maintain a minimal internal representation.

## Layers

**Lexing & Localization:**
- Purpose: Tokenize source and translate localized keywords.
- Location: `src/include/lexer.cpp`, `src/include/keywords.h`
- Contains: `Lexer` class and `KEYWORD_MAPPINGS`.
- Depends on: `keywords.h` for language-specific translation tables.
- Used by: `Parser`, `main.cpp`

**Parsing & AST:**
- Purpose: Transform tokens into an Abstract Syntax Tree.
- Location: `src/include/parser.cpp`, `src/include/alphabet_ast.h`
- Contains: Recursive descent parser and AST node definitions.
- Depends on: `Lexer`
- Used by: `Compiler`

**Compilation:**
- Purpose: Compile AST into stack-based bytecode.
- Location: `src/include/compiler.cpp`, `src/include/bytecode.h`
- Contains: `Compiler` class which traverses the AST and emits `OpCode` instructions.
- Depends on: `Parser`, `bytecode.h`
- Used by: `VM`, `main.cpp`

**Execution (VM):**
- Purpose: Execute bytecode instructions and manage state.
- Location: `src/include/vm.cpp`, `src/include/vm.h`
- Contains: `VM` class, stack management, and built-in function implementations (`z.*`).
- Depends on: `bytecode.h`, `ffi.h`, `type_system.h`
- Used by: `main.cpp`

## Data Flow

**Source to Execution:**

1. **Header Detection:** The `Lexer` reads the first line (e.g., `#alphabet<en>`) to set the translation language.
2. **Tokenization:** `Lexer` scans text, translating localized identifiers (like `si` in Spanish) to their internal counterparts (`i`).
3. **AST Generation:** `Parser` constructs a tree of `StmtPtr` and `ExprPtr` nodes.
4. **Bytecode Emission:** `Compiler` produces a `Program` object containing a vector of `Instruction` (OpCode + Operand).
5. **Stack Interpretation:** `VM` iterates through instructions, manipulating a value stack to perform computations.

**State Management:**
- **Stack-based:** Most operations use a LIFO stack for operands.
- **Environment Maps:** Globals and locals are stored in maps within the VM's call frames.
- **System Built-ins:** Prefixed with `z.` (e.g., `z.o` for output), handled directly by the VM.

## Key Abstractions

**Instruction:**
- Purpose: Represents a single unit of work for the VM.
- Examples: `src/include/bytecode.h`
- Pattern: OpCode + `std::variant` operand.

**Value:**
- Purpose: Represents any Alphabet data type (number, string, list, map, object).
- Examples: `src/include/vm.h` (implicitly via `Operand` and VM stack)
- Pattern: Tagged union or variant.

**TypeInfo:**
- Purpose: Metaprogramming and type safety metadata.
- Examples: `src/include/type_system.h`

## Entry Points

**CLI Interface:**
- Location: `src/main.cpp`
- Triggers: Execution of the `alphabet` binary from the command line.
- Responsibilities: Argument parsing, file reading, pipeline orchestration, REPL management, and LSP server startup.

**LSP Server:**
- Location: `src/include/lsp.cpp`
- Triggers: `--lsp` flag.
- Responsibilities: Provides language support features for IDEs.

## Error Handling

**Strategy:** Exception-based reporting for compile-time errors; runtime errors stop execution with stack traces.

**Patterns:**
- `ParseError`, `CompileError`, `RuntimeError` classes.
- Error synchronization in the parser to find multiple errors in one pass.

## Cross-Cutting Concerns

**Logging:** Standard output/error is used for most messages; bytecode dumping for debugging.
**Validation:** Type checker (`type_system.cpp`) validates operations before/during execution.
**FFI:** `ffi.cpp` provides a way to call native C functions from Alphabet code.

---

*Architecture analysis: 2025-02-13*
