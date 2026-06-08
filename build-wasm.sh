#!/bin/bash
# Alphabet Language — Emscripten (WebAssembly) Build Script
# Usage: ./build-wasm.sh

set -e

echo "=== Building Alphabet for WebAssembly ==="

# Check Emscripten
if ! command -v emcc &> /dev/null; then
    echo "Error: Emscripten not found."
    echo "Install: git clone https://github.com/emscripten-core/emsdk.git"
    echo "         cd emsdk && ./emsdk install latest && ./emsdk activate latest"
    echo "         source ./emsdk_env.sh"
    exit 1
fi

echo "Emscripten found: $(emcc --version | head -1)"

# Build
echo "Building..."
emcmake cmake -S . -B build-wasm \
    -DCMAKE_BUILD_TYPE=Release \
    -DCMAKE_EXE_LINKER_FLAGS="-s EXPORTED_FUNCTIONS=['_alphabet_run','_alphabet_eval','_alphabet_version','_malloc','_free']" \
    -DCMAKE_EXE_LINKER_FLAGS="-s EXPORTED_RUNTIME_METHODS=['ccall','cwrap']"

cmake --build build-wasm

echo ""
echo "=== Build Complete ==="
echo "Output: build-wasm/alphabet.js"
echo "Output: build-wasm/alphabet.wasm"
echo ""
echo "To use in browser:"
echo "  <script src='alphabet.js'></script>"
echo "  <script>"
echo "    Module.onRuntimeInitialized = () => {"
echo "      const run = Module.cwrap('alphabet_run', 'string', ['string']);"
echo "      const result = run('#alphabet<en>\\nz.o(\"Hello!\")');"
echo "      console.log(result);"
echo "    };"
echo "  </script>"
