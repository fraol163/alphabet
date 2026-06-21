#!/usr/bin/env bash
# Build the Alphabet Language Server as WASM for Node.js hosts.
# Mirrors build-wasm.sh but targets the LSP entry instead of the program runner.
#
# Requires: emcc (emsdk), cmake >= 3.16
#
# Produces: build-wasm-lsp/alphabet.wasm + alphabet.js
# Bundle target: editors/vscode-alphabet/server/wasm/

set -euo pipefail

cd "$(dirname "$0")"

# Activate emsdk if available (best-effort)
if [ -f "$HOME/emsdk/emsdk_env.sh" ]; then
    # shellcheck disable=SC1091
    source "$HOME/emsdk/emsdk_env.sh" >/dev/null 2>&1 || true
fi

if ! command -v emcc >/dev/null 2>&1; then
    echo "ERROR: emcc not found. Install emsdk:"
    echo "  cd \$HOME && git clone https://github.com/emscripten-core/emsdk"
    echo "  cd emsdk && ./emsdk install latest && ./emsdk activate latest"
    exit 1
fi

# Reconfigure cmake if VERSION changed (per alphabet-language-improvements skill)
VERSION=$(cat VERSION 2>/dev/null || echo "dev")
echo "Building Alphabet LSP WASM (version $VERSION)"

rm -rf build-wasm-lsp
emcmake cmake -S . -B build-wasm-lsp -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$(emcc -v 2>&1 | grep -oE '/[^ ]*emscripten\.cmake' | head -1)"

cmake --build build-wasm-lsp --target alphabet_lsp -j "$(nproc 2>/dev/null || echo 2)"

# Stage for the VS Code extension bundle
mkdir -p editors/vscode-alphabet/server/wasm
cp build-wasm-lsp/alphabet.js   editors/vscode-alphabet/server/wasm/alphabet.js
cp build-wasm-lsp/alphabet.wasm editors/vscode-alphabet/server/wasm/alphabet.wasm

echo
echo "Built: build-wasm-lsp/alphabet.wasm + alphabet.js"
echo "Staged: editors/vscode-alphabet/server/wasm/alphabet.{js,wasm}"
ls -la editors/vscode-alphabet/server/wasm/
