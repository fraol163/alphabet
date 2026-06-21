/**
 * WASM-backed Language Server runner for the Alphabet VS Code extension.
 *
 * Loads `alphabet.wasm` (built by `build-wasm-lsp.sh`) into Node's WebAssembly
 * runtime, wires stdio to LSP message streams, and runs the LSP loop via
 * the exported `_alphabet_lsp_main` entry.
 *
 * Lifecycle:
 *   const runner = new WasmLspRunner(context);
 *   await runner.start();
 *   runner.sendLspMessage(jsonRpcBytes);    // → stdin of WASM
 *   runner.onLspMessage((bytes) => { ... }); // ← stdout of WASM
 *   await runner.stop();
 *
 * I/O contract with WASM:
 *   - WASM std::cin reads come from `stdinQueue` (filled by sendLspMessage)
 *   - WASM std::cout writes go to `stdoutBuffer` (parsed for LSP frames
 *     and emitted via onLspMessage)
 *
 * Implementation notes:
 *   - The WASM binary is built with Emscripten MODULARIZE=1 and
 *     ENVIRONMENT=node (see CMakeListsWasmLsp.txt). This produces a
 *     `AlphabetLspModule` factory that we instantiate with options.
 *   - We use Emscripten's stdin/print callbacks rather than raw WASI
 *     fd_read/fd_write because they're simpler and Emscripten handles
 *     the WASI plumbing internally for us.
 *   - LSP framing: Content-Length: N\r\n\r\n{json of N bytes}. We
 *     buffer stdout until we see a complete frame, then emit.
 *
 * Requires:
 *   - alphabet.wasm + alphabet.js in <extension>/server/wasm/
 *   - Node 18+ (for stable WebAssembly + WASI)
 */

import * as fs from 'fs';
import * as path from 'path';
import { ExtensionContext } from 'vscode';

/** Minimal shape we need from the Emscripten MODULARIZE factory. */
export interface EmscriptenModule {
  // Lifecycle
  ready: Promise<this>;
  destroy?: (output?: unknown) => void;
  // Memory accessors (used if we ever need to pass a string)
  UTF8ToString?: (ptr: number, maxBytes?: number) => string;
  stringToUTF8?: (str: string, ptr: number, maxBytes: number) => number;
  // Exports we call
  _alphabet_lsp_main?: (argc: number, argv: number) => number;
}

/** Emscripten MODULARIZE factory signature. */
type EmscriptenFactory = (options: EmscriptenModuleOptions) => Promise<EmscriptenModule>;

export interface EmscriptenModuleOptions {
  // Emscripten stdio callbacks
  stdin?: () => number | null;        // return next byte (0-255) or null at EOF
  stdout?: (text: string) => void;    // Emscripten emits text via print
  stderr?: (text: string) => void;
  print?: (text: string) => void;
  printErr?: (text: string) => void;
  // Emscripten Module init options we use
  wasmBinary?: ArrayBufferView | ArrayBuffer; // pass pre-read .wasm bytes
  noInitialRun?: boolean;
  INITIAL_MEMORY?: number;
  ALLOW_MEMORY_GROWTH?: number;
  // Lifecycle hooks
  onAbort?: (what: unknown) => void;
  onExit?: (code: number) => void;
}

/** Public API for vscode-languageclient bridge. */
export interface WasmLspRunner {
  /** Load and instantiate the WASM module. Idempotent. */
  start(): Promise<void>;
  /** Push raw LSP request bytes (already Content-Length framed) into WASM stdin. */
  send(bytes: Buffer | Uint8Array): void;
  /** Register a handler for complete LSP response frames coming from WASM stdout. */
  onMessage(handler: (bytes: Buffer) => void): void;
  /** Tear down the WASM instance. */
  stop(): Promise<void>;
  /** True if start() succeeded and the instance is alive. */
  readonly isRunning: boolean;
}

/**
 * Locate the bundled WASM module and its Emscripten JS glue.
 * Path: <extensionRoot>/server/wasm/alphabet.{js,wasm}
 */
function resolveWasmPaths(context: ExtensionContext): { js: string; wasm: string } {
  const dir = path.join(context.extensionUri.fsPath, 'server', 'wasm');
  return {
    js: path.join(dir, 'alphabet.js'),
    wasm: path.join(dir, 'alphabet.wasm'),
  };
}

/**
 * Parse LSP frames out of a stdout byte stream.
 *
 * LSP framing: `Content-Length: N\r\n\r\n<body of N bytes>` repeated.
 * Returns a list of complete body buffers found, plus any leftover bytes
 * for the next call.
 *
 * Uses `unknown` casts to work around the Buffer<ArrayBuffer> vs
 * Buffer<ArrayBufferLike> variance mismatch in newer @types/node.
 */
function drainLspFrames(
  buffer: Buffer,
  frames: Buffer[],
): { remaining: Buffer } {
  const buf = buffer as unknown as Buffer<ArrayBuffer>;
  let pos = 0;
  while (pos < buf.length) {
    // Look for header end (\r\n\r\n)
    const headerEnd = buf.indexOf('\r\n\r\n', pos);
    if (headerEnd === -1) break;
    const header = buf.slice(pos, headerEnd).toString('ascii');
    const match = /Content-Length:\s*(\d+)/i.exec(header);
    if (!match) {
      // Malformed frame — skip 4 bytes and try again to recover
      pos = headerEnd + 4;
      continue;
    }
    const bodyLen = parseInt(match[1], 10);
    const bodyStart = headerEnd + 4;
    const bodyEnd = bodyStart + bodyLen;
    if (bodyEnd > buf.length) break; // wait for more data
    frames.push(buf.slice(bodyStart, bodyEnd) as unknown as Buffer);
    pos = bodyEnd;
  }
  return { remaining: (buf.slice(pos) as unknown) as Buffer };
}

/**
 * Concrete implementation of WasmLspRunner using Emscripten MODULARIZE output.
 */
class EmscriptenWasmLspRunner implements WasmLspRunner {
  private module: EmscriptenModule | undefined;
  private stdinQueue: number[] = [];
  // Carry buffer typed loosely to side-step Buffer<ArrayBufferLike> vs
  // Buffer<ArrayBuffer> variance under newer @types/node.
  private stdoutCarry: Buffer = Buffer.alloc(0) as unknown as Buffer;
  private messageHandler: ((bytes: Buffer) => void) | undefined;
  private _running = false;

  constructor(
    private readonly context: ExtensionContext,
    private readonly paths: { js: string; wasm: string },
    private readonly log: (msg: string) => void,
  ) {}

  get isRunning(): boolean {
    return this._running;
  }

  onMessage(handler: (bytes: Buffer) => void): void {
    this.messageHandler = handler;
  }

  send(bytes: Buffer | Uint8Array): void {
    for (let i = 0; i < bytes.length; i++) {
      this.stdinQueue.push(bytes[i]!);
    }
  }

  async start(): Promise<void> {
    if (this._running) return;
    this.log(`[wasm-lsp] loading ${this.paths.js}`);
    // Dynamic import — Emscripten's MODULARIZE output is a CommonJS-ish
    // factory that we invoke with options.
    // Use Function() to bypass TS static import resolution (path is dynamic).
    const factoryModuleId = this.paths.js;
    // eslint-disable-next-line @typescript-eslint/no-implied-eval
    const dynRequire: (id: string) => EmscriptenFactory = (0, eval)('require');
    const factory = dynRequire(factoryModuleId);

    const wasmBytes = await fs.promises.readFile(this.paths.wasm);
    this.log(`[wasm-lsp] read ${wasmBytes.length} bytes of WASM`);

    const moduleOptions: EmscriptenModuleOptions = {
      // Emscripten accepts the .wasm bytes as ArrayBufferView. Buffer is a
      // Uint8Array, which is an ArrayBufferView, but the strict types here
      // disallow the SharedArrayBuffer-backed variant in newer @types/node.
      // Cast through `unknown` to keep the call site honest.
      wasmBinary: wasmBytes as unknown as ArrayBufferView,
      stdin: () => this.pullStdinByte(),
      print: (text) => this.handleStdout(text),
      printErr: (text) => this.log(`[wasm-lsp stderr] ${text}`),
      noInitialRun: true,    // we'll call _alphabet_lsp_main manually
      ALLOW_MEMORY_GROWTH: 1,
      INITIAL_MEMORY: 32 * 1024 * 1024, // 32MB — LSP needs headroom for large projects
      onExit: (code) => {
        this.log(`[wasm-lsp] exited with code ${code}`);
        this._running = false;
      },
      onAbort: (what) => {
        this.log(`[wasm-lsp] ABORT: ${String(what)}`);
        this._running = false;
      },
    };

    this.module = await factory(moduleOptions);
    this._running = true;
    this.log('[wasm-lsp] module instantiated');

    // Manually invoke the LSP entry. We pass argc=0, argv=0 because the
    // LSP loop reads from std::cin regardless of argv (no file argument).
    // If we ever want to pass --lsp argv, we'd need to write to module memory.
    const exitCode = this.module._alphabet_lsp_main?.(0, 0);
    this.log(`[wasm-lsp] entry returned ${exitCode}`);
  }

  async stop(): Promise<void> {
    if (!this._running) return;
    // Emscripten has no clean shutdown — drop stdin EOF so getline returns false
    this.stdinQueue = []; // empty queue = null return = EOF on next read
    if (this.module?.destroy) {
      this.module.destroy();
    }
    this._running = false;
    this.log('[wasm-lsp] stopped');
  }

  // Emscripten stdin callback: return next byte or null at EOF.
  private pullStdinByte(): number | null {
    if (this.stdinQueue.length === 0) return null;
    return this.stdinQueue.shift()!;
  }

  // Emscripten print callback: invoked with stdout text chunks.
  // Multiple chunks may arrive per print call; parse for LSP frames.
  private handleStdout(text: string): void {
    if (text.length === 0) return;
    // Emscripten's `print` decodes UTF-8 bytes into a string; we need
    // bytes back. Re-encode — but for LSP (which is ASCII/JSON UTF-8)
    // round-trip is safe. Cast to Buffer<ArrayBuffer> for type compat
    // with newer @types/node where Buffer is generic over its backing buffer.
    const chunk = Buffer.from(text, 'utf8') as unknown as Buffer<ArrayBuffer>;
    this.stdoutCarry = Buffer.concat([this.stdoutCarry as unknown as Buffer<ArrayBuffer>, chunk]) as unknown as Buffer;
    const frames: Buffer[] = [];
    const { remaining } = drainLspFrames(this.stdoutCarry, frames);
    this.stdoutCarry = remaining;
    for (const frame of frames) {
      this.messageHandler?.(frame);
    }
  }
}

/**
 * Public factory. Returns null if WASM bundle not present.
 *
 * Internal: createWasmLspRunner does the actual work. tryStartWasmLsp is the
 * public name kept in sync with extension.ts import.
 */
async function createWasmLspRunner(
  context: ExtensionContext,
  log: (msg: string) => void,
): Promise<WasmLspRunner | null> {
  const paths = resolveWasmPaths(context);
  if (!fs.existsSync(paths.js) || !fs.existsSync(paths.wasm)) {
    log(`[wasm-lsp] bundle not found at ${paths.js} / ${paths.wasm}`);
    return null;
  }
  const runner = new EmscriptenWasmLspRunner(context, paths, log);
  try {
    await runner.start();
    return runner;
  } catch (err) {
    log(`[wasm-lsp] failed to start: ${err}`);
    return null;
  }
}

/** Public API name kept in sync with extension.ts import. */
export async function tryStartWasmLsp(
  context: ExtensionContext,
  log: (msg: string) => void,
): Promise<WasmLspRunner | null> {
  return createWasmLspRunner(context, log);
}

// Re-export createWasmLspRunner for direct use in tests.
export { createWasmLspRunner };
