# Changelog

All notable changes to the Alphabet VS Code extension are documented here. Format follows [Keep a Changelog](https://keepachangelog.com/), and this project adheres to [Semantic Versioning](https://semver.org/).

## [0.1.0] — 2026-06-21

### Added
- Initial release
- Multi-language syntax highlighting (en/am/de/es/fr) via TextMate grammar
- LSP client wrapping the bundled `alphabet --lsp` server
- Completion, hover, go-to-definition, document symbols, diagnostics
- Auto-detection of platform-specific bundled binary (linux-x64, darwin-x64, darwin-arm64, win32-x64, linux-arm64)
- Fallback to `alphabet` on `PATH` if bundled binary missing
- `alphabet.lsp.path` override setting
- Version handshake with warning on outdated binary
- 14 snippets (`hello`, `fn`, `class`, `if`, `for`, `while`, `try`, `match`, `import`, `const`, `main`, etc.)
- 9 commands: Run, Lint, Compile, Build WASM, NL-to-Code, Restart Server, Show Output, Open Docs, New File
- NL-to-Code sidebar webview (placeholder; real integration pending binary NL API)
- Run button in editor title bar (▶ icon)
- Configuration section with 7 settings (trace, format, lint, telemetry, etc.)
- Walkthrough notification on first install
- Folding markers, bracket auto-close, comment toggling

### Known limitations
- NL-to-Code panel uses placeholder responses until `alphabet nl` CLI is wired through
- No DAP debugger yet — breakpoints reserved for v0.5
- Windows ARM64 binary not yet shipped
