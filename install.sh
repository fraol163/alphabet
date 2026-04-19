#!/bin/sh
# Alphabet Language Installer
# One command: curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
#
# Tries pre-built release first. If not available, clones and builds from source.
# Always ends by starting the REPL.

set -e

REPO="fraol163/alphabet"
INSTALL_DIR="${ALPHABET_HOME:-/usr/local}"
BIN_DIR="$INSTALL_DIR/bin"
LIB_DIR="$INSTALL_DIR/share/alphabet"

# Uninstall
if [ "$1" = "--uninstall" ]; then
    echo "Uninstalling Alphabet..."
    sudo rm -f "$BIN_DIR/alphabet"
    sudo rm -rf "$LIB_DIR"
    echo "Done."
    exit 0
fi

# Detect platform
OS="$(uname -s)"
case "$OS" in
    Linux*)  PLATFORM="linux";  ARCHIVE="alphabet-linux-amd64.tar.gz" ;;
    Darwin*) PLATFORM="macos";  ARCHIVE="alphabet-macos-arm64.tar.gz" ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "Windows: download from https://github.com/$REPO/releases/latest"
        exit 0 ;;
    *) echo "Unsupported: $OS"; exit 1 ;;
esac

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

install_binary() {
    echo "  Installing to $BIN_DIR ..."
    sudo mkdir -p "$BIN_DIR" "$LIB_DIR"
    sudo cp "$1" "$BIN_DIR/alphabet"
    sudo chmod +x "$BIN_DIR/alphabet"
    [ -d "$2/stdlib" ] && sudo cp -r "$2/stdlib/"* "$LIB_DIR/" 2>/dev/null || true
}

start_repl() {
    echo ""
    echo "Alphabet is ready! Starting REPL..."
    echo ""
    exec "$BIN_DIR/alphabet" --repl
}

echo "Installing Alphabet Language ($PLATFORM)..."
echo ""

# Try pre-built release first
LATEST=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" 2>/dev/null | grep -o '"tag_name": *"[^"]*"' | cut -d'"' -f4 || true)

if [ -n "$LATEST" ]; then
    echo "Found release $LATEST, downloading..."
    URL="https://github.com/$REPO/releases/download/$LATEST/$ARCHIVE"
    if curl -fsSL "$URL" -o "$TMPDIR/alphabet.tar.gz" 2>/dev/null; then
        cd "$TMPDIR"
        tar xzf alphabet.tar.gz
        install_binary "$TMPDIR/alphabet" "$TMPDIR"
        start_repl
    fi
fi

# Fallback: build from source
echo "No pre-built release found. Building from source..."
echo ""

for cmd in git cmake g++ make; do
    command -v "$cmd" >/dev/null 2>&1 || { echo "Error: $cmd required. Install: sudo apt install $cmd"; exit 1; }
done

cd "$TMPDIR"
git clone --depth 1 "https://github.com/$REPO.git" alphabet
cd alphabet

echo "Compiling (this takes ~1 minute)..."
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -DCMAKE_MESSAGE_LOG_LEVEL=ERROR
cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2) -- -q

echo "Testing..."
cd build && ctest --output-on-failure --no-tests=error && cd ..

install_binary "$TMPDIR/alphabet/build/alphabet" "$TMPDIR/alphabet"
start_repl
