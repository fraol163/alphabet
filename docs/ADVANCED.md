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
15 libc = z.load_lib("libc.so.6")
libc.call("printf", "Hello from C!\n")
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
Source (.abc) → Lexer → Parser → Compiler → VM → Output
```

### Components

| Component | File | Purpose |
|-----------|------|---------|
| Lexer | src/include/lexer.h/cpp | Tokenization |
| Parser | src/include/parser.h/cpp | AST generation |
| Compiler | src/include/compiler.h/cpp | Bytecode |
| VM | src/include/vm.h/cpp | Execution |

---

## Memory Model

- **Stack:** 65536 slots (fixed, ~3MB)
- **Heap:** Dynamic (strings, lists, objects)

---

**More:** [Reference](REFERENCE.md) · [Source Code](../src/)
