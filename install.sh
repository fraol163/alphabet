#!/bin/sh
# Alphabet Language Installer
# curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
#
# Installs to ~/.local/bin (no sudo needed, works when piped)

set -e

REPO="fraol163/alphabet"
INSTALL_DIR="$HOME/.local"
BIN_DIR="$INSTALL_DIR/bin"
LIB_DIR="$INSTALL_DIR/share/alphabet"

# Uninstall
if [ "$1" = "--uninstall" ]; then
    echo "Uninstalling Alphabet..."
    rm -f "$BIN_DIR/alphabet"
    rm -rf "$LIB_DIR"
    echo "Alphabet uninstalled."
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

# Check if already installed
if command -v alphabet >/dev/null 2>&1; then
    CURRENT=$(alphabet --version 2>/dev/null | head -1 || echo "unknown")
    echo "  Already installed: $CURRENT"
    echo "  Updating..."
    echo ""
fi

install_binary() {
    echo "  Installing to $BIN_DIR/alphabet"
    mkdir -p "$BIN_DIR" "$LIB_DIR"
    cp "$1" "$BIN_DIR/alphabet"
    chmod 755 "$BIN_DIR/alphabet"
    [ -d "$2/stdlib" ] && cp -r "$2/stdlib/"* "$LIB_DIR/" 2>/dev/null || true
}

# Ensure ~/.local/bin is in PATH
ensure_path() {
    if ! echo "$PATH" | grep -q "$BIN_DIR"; then
        for PROFILE in "$HOME/.bashrc" "$HOME/.zshrc" "$HOME/.profile" "$HOME/.bash_profile"; do
            if [ -f "$PROFILE" ]; then
                if ! grep -q "$BIN_DIR" "$PROFILE" 2>/dev/null; then
                    echo "" >> "$PROFILE"
                    echo "# Alphabet Language" >> "$PROFILE"
                    echo "export PATH=\"\$HOME/.local/bin:\$PATH\"" >> "$PROFILE"
                    echo "  Added ~/.local/bin to PATH in $(basename $PROFILE)"
                fi
                return
            fi
        done
        # No profile found, create .profile
        echo "export PATH=\"\$HOME/.local/bin:\$PATH\"" > "$HOME/.profile"
        echo "  Created ~/.profile with PATH"
    fi
}

echo "Installing Alphabet Language ($PLATFORM)..."
echo ""

# Try pre-built release
LATEST=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" 2>/dev/null | grep -o '"tag_name": *"[^"]*"' | cut -d'"' -f4 || true)

if [ -n "$LATEST" ]; then
    echo "Downloading $LATEST..."
    URL="https://github.com/$REPO/releases/download/$LATEST/$ARCHIVE"
    if curl -fsSL "$URL" -o "$TMPDIR/alphabet.tar.gz" 2>/dev/null; then
        cd "$TMPDIR"
        tar xzf alphabet.tar.gz
        install_binary "$TMPDIR/alphabet" "$TMPDIR"
        ensure_path
        echo ""
        echo "Alphabet $LATEST installed!"
        "$BIN_DIR/alphabet" --version 2>/dev/null || true
        echo ""
        echo "Binary: $BIN_DIR/alphabet"
        echo ""
        echo "Open a new terminal or run: source ~/.bashrc"
        echo "Then: alphabet --repl"
        echo ""
        echo "Update:  curl -fsSL https://raw.githubusercontent.com/$REPO/main/install.sh | sh"
        echo "Remove:  curl -fsSL https://raw.githubusercontent.com/$REPO/main/install.sh | sh -s -- --uninstall"
        exit 0
    fi
fi

# Fallback: build from source
echo "Building from source (needs git, cmake, g++)..."
echo ""

for cmd in git cmake g++ make; do
    command -v "$cmd" >/dev/null 2>&1 || { echo "Error: $cmd not found. Install: sudo apt install $cmd"; exit 1; }
done

cd "$TMPDIR"
git clone --depth 1 "https://github.com/$REPO.git" alphabet
cd alphabet

echo "Compiling..."
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -DCMAKE_MESSAGE_LOG_LEVEL=ERROR
cmake --build build -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2) -- -q

echo "Testing..."
cd build && ctest --output-on-failure --no-tests=error && cd ..

install_binary "$TMPDIR/alphabet/build/alphabet" "$TMPDIR/alphabet"
ensure_path

echo ""
echo "Alphabet installed!"
"$BIN_DIR/alphabet" --version 2>/dev/null || true
echo ""
echo "Binary: $BIN_DIR/alphabet"
echo ""
echo "Open a new terminal or run: source ~/.bashrc"
echo "Then: alphabet --repl"
