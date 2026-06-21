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

# Find the Emscripten CMake toolchain explicitly. `emcc -v` no longer prints
# the path in emsdk >= 4, so we resolve it from the known emsdk layout.
EMSDK_TOOLCHAIN=""
for candidate in \
    "$HOME/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
    "/opt/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake" \
    "/usr/local/emsdk/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"; do
    if [ -f "$candidate" ]; then
        EMSDK_TOOLCHAIN="$candidate"
        break
    fi
done
if [ -z "$EMSDK_TOOLCHAIN" ]; then
    echo "ERROR: Could not locate Emscripten.cmake toolchain file."
    echo "Searched: $HOME/emsdk, /opt/emsdk, /usr/local/emsdk"
    exit 1
fi
echo "Using toolchain: $EMSDK_TOOLCHAIN"

rm -rf build-wasm-lsp
emcmake cmake -S editors/vscode-alphabet/build-tools/wasm-lsp -B build-wasm-lsp \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_TOOLCHAIN_FILE="$EMSDK_TOOLCHAIN"

cmake --build build-wasm-lsp --target alphabet_lsp -j "$(nproc 2>/dev/null || echo 2)"

# Stage for the VS Code extension bundle
mkdir -p editors/vscode-alphabet/server/wasm
cp build-wasm-lsp/alphabet.js   editors/vscode-alphabet/server/wasm/alphabet.js
cp build-wasm-lsp/alphabet.wasm editors/vscode-alphabet/server/wasm/alphabet.wasm

echo
echo "Built: build-wasm-lsp/alphabet.wasm + alphabet.js"
echo "Staged: editors/vscode-alphabet/server/wasm/alphabet.{js,wasm}"
ls -la editors/vscode-alphabet/server/wasm/
