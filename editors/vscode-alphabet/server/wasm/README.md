# WASM-backed LSP Transport — Implementation Plan

**Status:** Scaffolded (M2 of `alphabet-vscode-v1` goal). Real implementation requires the C++ LSP server to be compiled to WASM, which has not been done yet.

## Current State

The VS Code extension (`editors/vscode-alphabet/`) currently shells out to a native `alphabet` binary that must be installed on the user's machine. The goal is **zero-install**: open a `.abc` file in VS Code and LSP works without any binary on PATH.

The WASM build at `build-wasm/alphabet.wasm` (488K) only runs user programs — it is **not** the LSP server. The LSP server (`alphabet --lsp` mode) is part of the C++ binary.

## What Needs to Happen

### 1. Compile the C++ LSP server to WASM (blocked)

The current WASM build is a partial compile — it covers the VM and program runner but NOT the LSP server (`src/lsp.cpp`). To get true zero-install:

- Refactor `src/lsp.cpp` so its dependencies (fork/pthread/IO) can run under WASI
- Add `CMakeListsWasmLsp.txt` that targets the LSP entry point with `-s ENVIRONMENT=node`
- Resolve: LSP reads .abc files from disk → WASI virtual filesystem must be wired
- Resolve: `stdin` LSP messages → WASI `fd_read` from Node stdin
- Resolve: `stdout` LSP responses → WASI `fd_write` to Node stdout
- Test: ensure all LSP features (completion, hover, goto-def, diagnostics) work in WASM

### 2. Bundle WASM into the extension

- Add `server/wasm/alphabet.wasm` and `alphabet.js` to the extension
- Update `.vscodeignore` if needed (currently `node_modules/**` excluded; WASM goes elsewhere)
- Update `package.json` resources section if needed
- Total bundle size impact: +500KB to +2MB depending on WASM build flags

### 3. Wire `lsp-transport.ts` to run the WASM

The scaffold at `lsp-transport.ts` has:
- `pickTransport()` — decides wasm vs native vs PATH
- `resolveWasmBinary()` — finds bundled alphabet.wasm
- `createWasmLspRunner()` — STUB, needs real implementation

Real implementation outline:

```typescript
const wasmBytes = await fs.promises.readFile(binaryPath);
const wasmModule = await WebAssembly.compile(wasmBytes);

const stdinChunks: Buffer[] = [];
const stdoutChunks: Buffer[] = [];

const wasi = new WASI({
  args: ['alphabet', '--lsp'],
  env: { ...process.env, ALPHABET_LSP_CLIENT: 'vscode' },
  stdin: createStdin(stdinChunks),     // writable from extension
  stdout: createStdout(stdoutChunks), // readable to extension
  // stderr, fs, etc.
});

const instance = await WebAssembly.instantiate(wasmModule, {
  wasi_snapshot_preview1: wasi.wasiImport,
});

wasi.start(instance); // blocks; runs the LSP loop

// Pump stdin → instance
// Pump instance stdout → vscode-languageclient
```

### 4. Threading/FFI constraints (per alphabet-language-debugging skill)

The WASM build loses some capabilities:
- `z.thread`, `z.join`, `z.join_all`, `z.lock`, `z.acquire`, `z.release` (no pthreads)
- `z.dyn` FFI (no dlopen in WASI without extra work)
- File I/O bound to WASI virtual FS (current build uses Emscripten MEMFS)

These must be documented as "WASM fallback has reduced feature parity" so users understand.

### 5. Update user setting

Add `alphabet.transport` to `package.json`:
- `"wasm"` — force WASM
- `"native"` — force native binary
- `"auto"` (default) — prefer WASM, fall back to native

## Estimated Effort

| Step | Time |
|------|------|
| 1. C++ LSP → WASM compile | 3-5 sessions (largest blocker) |
| 2. Bundle WASM | 0.5 session |
| 3. Wire WASM transport | 1-2 sessions |
| 4. Threading/FFI docs | 0.5 session |
| 5. Settings UI | 0.5 session |

**Total:** ~6-9 sessions for full M2.

## Quick Win Alternative

While waiting for full LSP→WASM, ship a **WASM-only "Run" command** that uses the existing `build-wasm/alphabet.wasm` for program execution only. LSP features still use the bundled native binary, but "Run File" can work zero-install. This gives 80% of the zero-install benefit with 20% of the work.

See `references/web-ide-v235-integration.md` in the alphabet-language-debugging skill for the existing browser-WASM pattern that could be adapted.
