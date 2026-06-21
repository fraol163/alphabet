# Alphabet Language — VS Code Extension

Official VS Code support for the [Alphabet](https://github.com/fraol163/Alphabet_Language) programming language (v2.3.5).

## ✨ Zero-install: works without the `alphabet` binary

The extension ships the Language Server as **357 KB of WebAssembly** in the `.vsix`. Users do **not** need to install the `alphabet` binary, a C++ toolchain, or any local build tooling. Open any `.abc` file and the LSP works.

If a native binary is present on `PATH` (or `alphabet.lsp.path` is set), the extension will use it for slightly faster cold-start and full feature parity. Otherwise it transparently falls back to the bundled WASM.

Settings:
- `alphabet.transport` — `auto` (default, prefer WASM), `wasm` (force WASM), `native` (force binary)

## Features

| | Feature | Backend |
|---|---|---|
| 🌍 | **Multi-language syntax** — keywords in English, Amharic (`ክፍል`), Spanish (`clase`), French (`classe`), German (`klasse`) | TextMate grammar |
| 📝 | **Completion** — trigger on space/dot/`(`/newline, with `editRange` resolve support | LSP `completionProvider` |
| 📖 | **Hover docs** — inline documentation for builtins, classes, methods | LSP `hoverProvider` |
| 🔗 | **Go-to-definition** — jump from identifier to its declaration | LSP `definitionProvider` |
| 🔍 | **Find references** — every occurrence of a symbol | LSP `referencesProvider` |
| ✏️ | **Rename symbol** — workspace-aware refactor | LSP `renameProvider` |
| 📋 | **Document outline** — symbols panel for the file | LSP `documentSymbolProvider` |
| 💡 | **Signature help** — parameter hints triggered on `(` and `,` | LSP `signatureHelpProvider` |
| 🔧 | **Quick fixes** — `codeAction` for every diagnostic | LSP `codeActionProvider` |
| 🔆 | **Document highlight** — all occurrences of the word at cursor | LSP `documentHighlightProvider` |
| 🎨 | **Semantic tokens** — modern syntax coloring (keywords/strings/numbers/types/operators) | LSP `semanticTokensProvider` (10 token types, full + range, delta-encoded) |
| 📌 | **Inlay hints** — inline parameter names at call sites | LSP `inlayHintProvider` |
| 🛠 | **Format on save** — full-document + range formatting | LSP `documentFormattingProvider` + `documentRangeFormattingProvider` |
| 🔍 | **Real-time diagnostics** — parse + lint errors as you type | LSP `textDocument/publishDiagnostics` |
| 🎨 | **Syntax highlighting** | TextMate grammar (en/am/de/es/fr) |
| 🚀 | **Run button** — one-click `.abc` execution | VS Code command |
| 📦 | **Compile to bytecode** + **WASM build** | VS Code commands |
| 💬 | **NL-to-Code panel** — sidebar webview | Webview |
| 🧩 | **Snippets** — `fn`, `class`, `if`, `for`, `try`, `match`, `main` | VS Code snippets |

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
| `alphabet.transport` | `auto` | `auto` / `wasm` / `native` — picks LSP transport |
| `alphabet.lsp.path` | `null` | Path to native `alphabet` binary (overrides transport) |
| `alphabet.lsp.trace` | `off` | LSP trace level: `off` / `messages` / `verbose` |
| `alphabet.format.enable` | `true` | Format `.abc` on save |
| `alphabet.lint.enable` | `true` | Run linter and show diagnostics |
| `alphabet.lint.severity` | `warning` | Minimum diagnostic severity |
| `alphabet.run.command` | `alphabet run` | Command run by the play button |
| `alphabet.compile.command` | `alphabet -c -o` | Command run by the compile button |
| `alphabet.buildWasm.command` | `alphabet build-wasm` | Command run by the WASM build button |
| `alphabet.completion.engine` | `lsp` | `lsp` or `lsp+nl` (experimental NL augmentation) |
| `alphabet.telemetry.enabled` | `false` | Opt-in anonymous usage stats |
| `alphabet.files.associations` | `**/*.abc` | Glob patterns for files to treat as Alphabet |

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
| `Alphabet: Show Getting Started` | — | Open the walkthrough |

## How zero-install works

```
                ┌─────────────────────────────────────┐
                │   VS Code Extension (.vsix)         │
                │  ┌───────────────────────────────┐  │
                │  │  dist/extension.js (TS+esbuild)│ │
                │  │  server/wasm/alphabet.js       │ │
                │  │  server/wasm/alphabet.wasm     │ │
                │  │    (LSP server compiled to     │ │
                │  │     WebAssembly — 357 KB)       │ │
                │  └───────────────────────────────┘  │
                └─────────────────────────────────────┘
                              │
                              ▼
   User opens hello.abc → extension activates
   → tryStartWasmLsp() loads alphabet.wasm via Node WebAssembly
   → vscode-languageclient talks to in-process WASM LSP
   → full features (completion, hover, semantic tokens) without binary
```

If WASM fails to load (corrupted bundle, missing file), the extension falls back to spawning the native `alphabet` binary if available, then to PATH lookup. The extension NEVER silently breaks — it shows a clear error.

## Troubleshooting

### The extension loads but completion/hover don't work

1. Open **View → Output → Alphabet Language Server** to see init logs
2. Check whether the LSP started (look for "Alphabet language server started" or "Using WASM Language Server")
3. If you see "Method not found" errors, your binary/WASM is older than the extension expects — update with `alphabet update --force` or reinstall the extension

### I want to force the native binary

Set `alphabet.transport: native` and `alphabet.lsp.path: /absolute/path/to/alphabet`.

### WASM initialization fails

The extension will fall back to the native binary automatically. If you want to debug, set `alphabet.lsp.trace: verbose`.

## Development

### Build the WASM LSP server locally

```bash
# Install emsdk (~500 MB)
git clone https://github.com/emscripten-core/emsdk ~/emsdk
cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh

# Build
cd /path/to/Alphabet_Language
./build-wasm-lsp.sh

# Run smoke test
node editors/vscode-alphabet/server/wasm/smoke-test.mjs
```

### Package the .vsix

```bash
cd editors/vscode-alphabet
npm install
npm run build  # esbuild → dist/extension.js
npx @vscode/vsce package --no-dependencies
```

### CI

`.github/workflows/vscode-extension-release.yml` builds the WASM and packages the VSIX on every release tag.
