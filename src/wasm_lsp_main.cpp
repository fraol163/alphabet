/**
 * WASM-targeted entry point for the Alphabet Language Server.
 *
 * Built by `CMakeListsWasmLsp.txt` with emcc. Exports `_alphabet_lsp_main`
 * so the Node.js host (editors/vscode-alphabet/server/wasm/lsp-transport.ts)
 * can call it from WASI after wiring stdin/stdout.
 *
 * Behavior:
 *   - Constructs the LSP server
 *   - Calls server.run(), which loops on std::getline(std::cin) for LSP
 *     messages and writes JSON-RPC responses to std::cout
 *   - Returns 0 on clean shutdown, non-zero on error
 *
 * I/O contract:
 *   - stdin: LSP request frames (Content-Length headers + JSON body)
 *   - stdout: LSP response frames (Content-Length headers + JSON body)
 *   - stderr: LSP log messages (written via std::cerr)
 *
 * The host (Node + WASI) must wire these to the extension's stdio so
 * vscode-languageclient can talk to the LanguageServer just like a
 * native binary subprocess.
 *
 * Constraints:
 *   - std::cin/std::cout in WASI come from fd 0/1
 *   - Node-flavored emcc build with `-s ENVIRONMENT=node` is required
 *   - No threads, no filesystem (LSP reads files via wasi::fd_read which
 *     the host must passthrough to host filesystem)
 */

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <string>

#include "lsp.h"

extern "C" {

/**
 * Entry point invoked from Node via WASI after stdio is wired.
 *
 * Argv semantics (passed by host via wasi_snapshot_preview1::args_get):
 *   argv[0]: "alphabet" (program name)
 *   argv[1]: "--lsp"    (forces LSP mode)
 *
 * Returns 0 on clean shutdown.
 */
int alphabet_lsp_main(int argc, char** argv) {
    // Suppress any C++ exception leaking out — the LSP loop catches its own
    // errors but a bug in setup must not crash the WASM instance.
    try {
        alphabet::lsp::LanguageServer server;
        server.run();
    } catch (const std::exception& e) {
        std::fprintf(stderr, "alphabet_lsp_main: fatal: %s\n", e.what());
        return 1;
    } catch (...) {
        std::fprintf(stderr, "alphabet_lsp_main: fatal: unknown exception\n");
        return 1;
    }
    return 0;
}

} // extern "C"
