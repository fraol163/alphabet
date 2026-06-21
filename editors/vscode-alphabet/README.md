# Alphabet Language — VS Code Extension

Official VS Code support for the [Alphabet](https://github.com/yourusername/Alphabet_Language) programming language.

## Features

- 🌍 **Multi-language syntax** — Write keywords in English, Amharic (`ክፍል`), Spanish (`clase`), French (`classe`), or German (`klasse`)
- ⚡ **Language Server Protocol** — Completion, hover docs, go-to-definition, document symbols, real-time diagnostics
- 🎨 **Syntax highlighting** — Full TextMate grammar covering en/am/de/es/fr keywords
- 🚀 **Run button** — One-click `.abc` execution
- 🔧 **Compile & lint** — Direct integration with the `alphabet` binary
- 📦 **WASM build** — One-click WebAssembly build pipeline
- 💬 **NL-to-Code panel** — Sidebar webview for natural-language → Alphabet code (placeholder)
- 🔍 **Snippets** — `fn`, `class`, `if`, `for`, `try`, `match`, `main`, etc.

## Installation

### From VS Code Marketplace

1. Open VS Code
2. Press `Ctrl+P` / `Cmd+P`, type `ext install alphabet`
3. Click **Install**

Or search "Alphabet Language" in the Extensions sidebar.

### From VSIX

```bash
code --install-extension fraolteshome.alphabet-<version>.vsix
```

### Uninstall

Same as any other extension:
- Extensions sidebar → right-click "Alphabet Language" → **Uninstall**
- Or `code --uninstall-extension fraolteshome.alphabet`

## Quick Start

```alphabet
#alphabet<en>

m 0 main() {
  z.o("Hello, Alphabet!")
}
```

1. Create a new file: `Ctrl+N` → save as `hello.abc`
2. Or run the command `Alphabet: New .abc File` from the palette
3. Press the **▶** run button in the editor title bar

## Configuration

| Setting | Default | Description |
|---|---|---|
| `alphabet.lsp.path` | `null` | Path to `alphabet` binary. Empty = bundled binary. |
| `alphabet.lsp.trace` | `off` | LSP trace level: `off` / `messages` / `verbose` |
| `alphabet.format.enable` | `true` | Format `.abc` on save |
| `alphabet.lint.enable` | `true` | Run linter and show diagnostics |
| `alphabet.lint.severity` | `warning` | Minimum diagnostic severity |
| `alphabet.run.command` | `alphabet run` | Command run by the play button |
| `alphabet.completion.engine` | `lsp` | `lsp` or `lsp+nl` (experimental NL augmentation) |
| `alphabet.telemetry.enabled` | `false` | Opt-in anonymous usage stats |

## Commands

| Command | Shortcut | Description |
|---|---|---|
| `Alphabet: Run File` | Editor title ▶ button | Run current `.abc` |
| `Alphabet: Lint File` | — | Run linter on current file |
| `Alphabet: Compile to Bytecode` | — | Compile `.abc` to `.abc.bc` |
| `Alphabet: Build WASM` | — | Trigger WASM build pipeline |
| `Alphabet: Open NL-to-Code Panel` | — | Open sidebar webview |
| `Alphabet: Restart Language Server` | — | Restart LSP |
| `Alphabet: Show Server Output` | — | Show extension output |
| `Alphabet: Open Documentation` | — | Open GitHub README |
| `Alphabet: New .abc File` | — | Scaffold a new file |

## Troubleshooting

### Binary not found

The extension bundles a prebuilt `alphabet` binary for your platform. If missing:
1. Install Alphabet from source: `git clone https://github.com/yourusername/Alphabet_Language && cd Alphabet_Language && cmake -B build && cmake --build build`
2. Either put `alphabet` on your `PATH`, or set `alphabet.lsp.path` in VS Code settings

### Server not starting

Open the output panel:
- `View → Output → Alphabet Language Server`

Common causes:
- Wrong architecture binary (x64 vs ARM64)
- Missing system libraries (e.g. `libstdc++` on older Linux)
- Antivirus blocking the binary

### Version mismatch

The extension checks the binary version on activation. If too old:
```
alphabet update --force
```

## Architecture

```
VS Code Extension (TypeScript)
  ├── extension.ts          ← LSP client wrapper
  ├── nl-to-code.ts         ← Webview panel
  ├── syntaxes/*.tmLanguage.json
  ├── language-configuration.json
  └── snippets/alphabet.json

Alphabet Binary (C++17, bundled)
  ├── src/lsp.cpp           ← LSP server (reuse as-is)
  └── src/main.cpp          ← --lsp entry point
```

All language intelligence flows through the existing C++ LSP server. The TypeScript client only:
1. Spawns the binary
2. Forwards JSON-RPC over stdio
3. Maps responses to VS Code UI

## License

MIT
