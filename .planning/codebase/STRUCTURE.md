# Codebase Structure

**Analysis Date:** 2025-02-13

## Directory Layout

```
[project-root]/
├── src/                # C++ source code for the compiler and VM
│   ├── main.cpp        # CLI entry point and orchestration
│   └── include/        # Header and implementation files for core logic
├── stdlib/             # Alphabet source files for the standard library
├── tests/              # C++ unit tests and Alphabet integration tests
├── docs/               # Technical documentation and guides
├── examples/           # Sample Alphabet programs
├── third_party/        # External libraries (e.g., JSON header)
├── tooling/            # Editor support (VS Code grammar)
├── CMakeLists.txt      # Build system configuration
└── VERSION             # Single source of truth for versioning
```

## Directory Purposes

**src/:**
- Purpose: Core implementation of the language.
- Contains: C++ source files for lexing, parsing, compiling, and executing.
- Key files: `src/main.cpp`, `src/include/vm.cpp`, `src/include/compiler.cpp`.

**stdlib/:**
- Purpose: Standard library written in Alphabet.
- Contains: Modules for I/O, list manipulation, math, and string operations.
- Key files: `stdlib/io.abc`, `stdlib/list.abc`.

**tests/:**
- Purpose: Quality assurance.
- Contains: C++ unit tests for components and "golden" tests that compare program output.
- Key files: `tests/test_vm.cpp`, `tests/golden_test_runner.sh`.

**docs/:**
- Purpose: Project documentation.
- Contains: Tutorials, reference guides, and architecture diagrams.

**tooling/:**
- Purpose: Development experience.
- Contains: Syntax highlighting grammars for editors.

## Key File Locations

**Entry Points:**
- `src/main.cpp`: Main CLI that handles compilation and execution.
- `src/include/lsp.cpp`: Language Server Protocol entry point.

**Configuration:**
- `CMakeLists.txt`: Build and installation configuration.
- `.clang-format`: Code style configuration.

**Core Logic:**
- `src/include/lexer.cpp`: Localized tokenization.
- `src/include/parser.cpp`: AST construction.
- `src/include/compiler.cpp`: Bytecode generation.
- `src/include/vm.cpp`: Bytecode execution.

**Testing:**
- `tests/golden_files/`: Directory containing `.abc` test cases and `.expected` outputs.

## Naming Conventions

**Files:**
- C++ Source: `.cpp` and `.h`
- Alphabet Source: `.abc`
- Test Files: `test_*.cpp` or `*.abc` with corresponding `*.expected`

**Directories:**
- Snake case for most directories (e.g., `third_party`, `golden_files`).

## Where to Add New Code

**New Feature (Language):**
- Implementation: `src/include/` (Lexer/Parser/Compiler/VM as needed)
- Tests: `tests/golden_files/new_feature.abc` and `tests/golden_files/new_feature.expected`

**New Standard Library Function:**
- Implementation: Relevant `.abc` file in `stdlib/`
- Built-in backing: `src/include/vm.cpp` (if a new `z.*` function is needed)

**Utilities:**
- Shared helpers: `src/include/`

## Special Directories

**build/:**
- Purpose: Compilation artifacts and binaries.
- Generated: Yes
- Committed: No

**.planning/:**
- Purpose: Architecture and structure analysis for GSD.
- Generated: Yes
- Committed: No (usually)

---

*Structure analysis: 2025-02-13*
