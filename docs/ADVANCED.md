# Advanced Features

**Deep dive into Alphabet's advanced features.**

---

## Table of Contents

1. [Foreign Function Interface (FFI)](#foreign-function-interface-ffi)
2. [Language Server Protocol (LSP)](#language-server-protocol-lsp)
3. [Architecture](#architecture)
4. [Memory Model](#memory-model)

---

## Foreign Function Interface (FFI)

Call external C/C++ libraries:

```alphabet
#alphabet<en>
5 result = z.dyn("/path/to/lib.so", "add", 10, 20)
z.o(result)  # Output: 30
```

---

## Language Server Protocol (LSP)

Real-time error highlighting in VS Code:

```bash
alphabet --lsp
```

### VS Code Setup

Add to `.vscode/settings.json`:

```json
{
  "alphabet.lsp.command": "alphabet",
  "alphabet.lsp.args": ["--lsp"]
}
```

---

## Architecture

```
Source (.abc) → Lexer → Tokens → Parser → AST → Compiler → Bytecode → VM → Output
```

### Components

| Component | File | Purpose |
|-----------|------|---------|
| Lexer | src/lexer.cpp + src/include/lexer.h | Tokenization |
| Parser | src/parser.cpp + src/include/parser.h | AST generation |
| Compiler | src/compiler.cpp + src/include/compiler.h | Bytecode |
| VM | src/vm.cpp + src/vm_builtins.cpp + src/include/vm.h | Execution |

---

## Memory Model

- **Stack:** Fixed-size array (STACK_MAX=65536) with unique_ptr
- **Heap:** Dynamic (strings, lists, objects)

---

**More:** [Reference](REFERENCE.md) · [Source Code](../src/)
