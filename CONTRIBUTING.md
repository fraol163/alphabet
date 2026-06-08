# Contributing to Alphabet Language

## Getting Started

```bash
git clone https://github.com/fraol163/alphabet.git
cd alphabet
cmake -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

## Project Structure

```
src/
  lexer.cpp          — Tokenizer (source → tokens)
  parser.cpp         — Parser (tokens → AST)
  compiler.cpp       — Compiler (AST → bytecode)
  vm.cpp             — Virtual machine (bytecode → results)
  vm_builtins.cpp    — 70+ built-in functions
  main.cpp           — CLI, REPL, debugger, update
  lsp.cpp            — Language Server Protocol for VS Code
  ffi.cpp            — Foreign Function Interface
  type_system.cpp    — Type checking
src/include/
  ast.h              — AST node definitions
  bytecode.h         — Opcode definitions
  *.h                — Headers for each module
tests/
  test_*.cpp         — Unit tests (Google Test)
  golden/            — Golden file tests for all 5 languages
stdlib/
  *.abc              — Standard library modules
```

## Code Style

- C++17 standard
- 4-space indentation
- `snake_case` for functions and variables
- `PascalCase` for types and classes
- `UPPER_CASE` for constants and opcodes
- No comments unless explaining non-obvious logic

## Adding a Builtin Function

1. Add handler in `vm_builtins.cpp`:
```cpp
Value my_func(VM &vm, const std::vector<Value> &args) {
    // implementation
    return result;
}
```

2. Register in `register_builtins()`:
```cpp
builtins["my_func"] = my_func;
```

3. Add to `BUILTIN_NAMES` in `vm.cpp` for shorthand access.

4. Add doc entry in `main.cpp` `alphabet doc` command.

5. Add test in `tests/test_vm.cpp`.

## Adding an Opcode

1. Define in `bytecode.h`:
```cpp
MY_OP = 52,
```

2. Add handler in `vm.cpp`:
```cpp
case Opcode::MY_OP: {
    // implementation
    break;
}
```

3. Add compiler emit in `compiler.cpp`.

4. Add to disassembler in `vm.cpp` `disassemble()`.

## Testing

```bash
cd build && ctest --output-on-failure
```

All 91 tests must pass before submitting a PR.

## Pull Requests

1. Fork the repo
2. Create a feature branch (`git checkout -b feature/my-feature`)
3. Make changes
4. Run tests (`ctest --output-on-failure`)
5. Submit PR with description of changes

## Reporting Bugs

Open an issue with:
- Alphabet version (`alphabet --version`)
- Operating system
- Minimal code that reproduces the bug
- Expected vs actual output

## Language Design Principles

1. **Multilingual first** — every keyword must be translatable
2. **Simple syntax** — single-letter keywords for common constructs
3. **Fast feedback** — compile errors should be immediate and clear
4. **Safe by default** — sandbox mode, memory limits, stack overflow protection
5. **Teaching language** — code should be readable by non-programmers
