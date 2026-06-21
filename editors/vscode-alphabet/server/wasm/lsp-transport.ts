/**
 * WASM-backed Language Server transport for the Alphabet VS Code extension.
 *
 * Goal: spawn `alphabet.wasm` (built from build-wasm/) inside the extension
 * host using Node's `WebAssembly` + `WASI` APIs, run it with `--lsp` argv,
 * and bridge its stdio to vscode-languageclient over stdin/stdout.
 *
 * Status: SCAFFOLD ONLY — the C++ LSP server has not yet been compiled to
 * WASM (only the program-runner VM has been). When C++ LSP→WASM happens,
 * this module wires it up.
 *
 * Why this exists: the extension currently shells out to a native binary
 * (`alphabet --lsp`). For true zero-install (no binary on PATH), the LSP
 * server itself must run inside WASM. The C++ source is portable; emcc can
 * compile it. Once compiled, this bridge makes the WASM binary look like
 * a normal LSP child process to vscode-languageclient.
 *
 * Known constraints (from alphabet-language-debugging skill notes):
 *   - WASM build cannot use pthreads/threading (z.thread etc. fail)
 *   - FFI (z.dyn) requires dlopen which WASI doesn't expose by default
 *   - REPL `read_file`/`write_file` need virtual filesystem passthrough
 *   - Existing build-wasm/ artifacts target browser, not Node — need a
 *     Node-friendly emcc build with `-s ENVIRONMENT=node`
 *
 * @see ../extension.ts resolveServerBinary for the transport selection
 */

import { spawn } from 'child_process';
import * as fs from 'fs';
import * as path from 'path';
import { ExtensionContext } from 'vscode';

export type LspTransport = 'native' | 'wasm' | 'wasm-fallback-native';

/**
 * Decide which LSP transport to use based on:
 *   1. User setting `alphabet.transport` (override)
 *   2. Bundled WASM availability + Node version >= 18
 *   3. Bundled native binary
 *   4. alphabet on PATH
 */
export async function pickTransport(
  context: ExtensionContext,
  userSetting: string | undefined,
): Promise<{ transport: LspTransport; command: string; args: string[] }> {
  // 1. User override
  if (userSetting === 'wasm') {
    const wasmPath = resolveWasmBinary(context);
    if (await exists(wasmPath.binary)) {
      return { transport: 'wasm', ...wasmPath };
    }
    throw new Error(`alphabet.transport=wasm requested but WASM not bundled at ${wasmPath.binary}`);
  }
  if (userSetting === 'native') {
    const nativePath = resolveNativeBinary(context);
    if (await exists(nativePath.binary)) {
      return { transport: 'native', ...nativePath };
    }
    throw new Error(`alphabet.transport=native requested but no bundled binary`);
  }

  // 2. Try WASM first (zero-install promise)
  if (await canUseWasm(context)) {
    return { transport: 'wasm', ...resolveWasmBinary(context) };
  }

  // 3. Fall back to native binary
  const native = resolveNativeBinary(context);
  if (await exists(native.binary)) {
    return { transport: 'wasm-fallback-native', ...native };
  }

  // 4. PATH
  const onPath = findOnPath('alphabet');
  if (onPath) {
    return { transport: 'wasm-fallback-native', command: onPath, args: ['--lsp'] };
  }

  throw new Error('No alphabet runtime available (no WASM, no bundled binary, no PATH entry)');
}

function resolveWasmBinary(context: ExtensionContext): { binary: string; args: string[] } {
  const wasmDir = path.join(context.extensionUri.fsPath, 'server', 'wasm');
  // TODO: bundle alphabet.wasm + alphabet.js (Node-flavored) into server/wasm/
  const binary = path.join(wasmDir, 'alphabet.wasm');
  const args = ['--lsp'];
  return { binary, args };
}

function resolveNativeBinary(context: ExtensionContext): { binary: string; args: string[] } {
  const platform = process.platform;
  const arch = process.arch;
  const isWin = platform === 'win32';
  const ext = isWin ? '.exe' : '';
  const platformName =
    platform === 'darwin' ? 'darwin' :
    platform === 'win32'  ? 'win32'  :
                             'linux';
  const archName = arch === 'arm64' ? 'arm64' : 'x64';
  const filename = `alphabet-${platformName}-${archName}${ext}`;
  const binary = path.join(context.extensionUri.fsPath, 'server', 'bin', filename);
  return { binary, args: ['--lsp'] };
}

async function exists(p: string): Promise<boolean> {
  try { await fs.promises.access(p, fs.constants.X_OK); return true; }
  catch { return false; }
}

async function canUseWasm(_context: ExtensionContext): Promise<boolean> {
  // TODO: gate on:
  //   - Node >= 18 (has stable WebAssembly + WASI)
  //   - bundled alphabet.wasm exists
  //   - the C++ LSP server has actually been compiled to WASM (it hasn't yet)
  return false;
}

function findOnPath(name: string): string | undefined {
  const PATH = process.env.PATH ?? '';
  const sep = path.delimiter;
  const exts = process.platform === 'win32'
    ? (process.env.PATHEXT ?? '.EXE;.CMD;.BAT').split(';')
    : [''];
  for (const dir of PATH.split(sep)) {
    if (!dir) continue;
    for (const e of exts) {
      const candidate = path.join(dir, name + e);
      if (fs.existsSync(candidate)) return candidate;
    }
  }
  return undefined;
}

/**
 * Future: when WASM LSP is available, run it in-process via WebAssembly + WASI.
 * This stub documents the shape so the next session can fill it in.
 */
export interface WasmLspRunner {
  start(): Promise<void>;
  stop(): Promise<void>;
  onMessage(handler: (msg: string) => void): void;
  send(msg: string): void;
}

/**
 * PLACEHOLDER — real implementation will:
 *   1. Read alphabet.wasm bytes
 *   2. Compile via WebAssembly.compile (or instantiateStreaming if file://)
 *   3. Wire WASI imports (fd_write → stdout capture, fd_read ← stdin)
 *   4. Call `_alphabet_lsp_main(argc, argv)` via exports
 *   5. Pump stdin from vscode-languageclient into WASI fd_read
 *   6. Pump WASI fd_write output back to vscode-languageclient
 *
 * Known blockers before this can be implemented:
 *   1. alphabet C++ LSP server must be compilable to WASM (it uses fork/pthread/IO
 *      heavily — may need refactor)
 *   2. existing build-wasm/ target is browser; need Node-flavored build
 *   3. WASI preview1 vs preview2 choice (Node 18+ supports both)
 */
export async function createWasmLspRunner(_binaryPath: string): Promise<WasmLspRunner> {
  throw new Error(
    'WASM LSP runner not yet implemented. ' +
    'Requires alphabet C++ LSP server to be compiled to WASM first. ' +
    'See server/wasm/README.md for the implementation plan.',
  );
}

// Unused spawn import — kept to avoid unused-import warnings when the real
// implementation lands and needs child_process.
void spawn;
