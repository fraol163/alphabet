#!/bin/sh
# Alphabet Language Installer
# curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
#
# Installs to ~/.local/bin (no sudo needed, works when piped)
# Supports: Linux (x86_64), macOS (arm64/x86_64)

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
    hash -r 2>/dev/null || true
    echo "Alphabet uninstalled."
    exit 0
fi

# Detect platform and architecture
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Linux*)
        case "$ARCH" in
            x86_64|amd64)  PLATFORM="linux"; ARCHIVE="alphabet-linux-amd64.tar.gz" ;;
            *) echo "Unsupported Linux architecture: $ARCH (only x86_64 supported)"; exit 1 ;;
        esac
        ;;
    Darwin*)
        case "$ARCH" in
            arm64|aarch64) PLATFORM="macos"; ARCHIVE="alphabet-macos-arm64.tar.gz" ;;
            x86_64)        PLATFORM="macos"; ARCHIVE="alphabet-macos-arm64.tar.gz" ;;  # Rosetta
            *) echo "Unsupported macOS architecture: $ARCH"; exit 1 ;;
        esac
        ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "Windows detected."
        echo "Download from: https://github.com/$REPO/releases/latest"
        exit 0 ;;
    *)
        echo "Unsupported OS: $OS"
        echo "Supported: Linux (x86_64), macOS (arm64/x86_64)"
        exit 1 ;;
esac

TMPDIR=$(mktemp -d)
trap 'rm -rf "$TMPDIR"' EXIT

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
    if [ -d "$2/stdlib" ]; then
        cp -r "$2/stdlib/"* "$LIB_DIR/" 2>/dev/null || true
    fi
}

# Ensure ~/.local/bin is in PATH
ensure_path() {
    hash -r 2>/dev/null || true

    if echo "$PATH" | grep -q "$BIN_DIR"; then
        return
    fi

    # macOS default shell is zsh, Linux is usually bash
    if [ "$OS" = "Darwin" ]; then
        PROFILES="$HOME/.zshrc $HOME/.bash_profile $HOME/.profile"
    else
        PROFILES="$HOME/.bashrc $HOME/.bash_profile $HOME/.zshrc $HOME/.profile"
    fi

    for PROFILE in $PROFILES; do
        if [ -f "$PROFILE" ]; then
            if ! grep -q "$BIN_DIR" "$PROFILE" 2>/dev/null; then
                echo "" >> "$PROFILE"
                echo "# Alphabet Language" >> "$PROFILE"
                echo "export PATH=\"\$HOME/.local/bin:\$PATH\"" >> "$PROFILE"
                echo "  Added ~/.local/bin to PATH in $(basename "$PROFILE")"
            fi
            return
        fi
    done

    # No profile found, create one based on OS
    if [ "$OS" = "Darwin" ]; then
        PROFILE="$HOME/.zshrc"
    else
        PROFILE="$HOME/.profile"
    fi
    echo "export PATH=\"\$HOME/.local/bin:\$PATH\"" > "$PROFILE"
    echo "  Created $(basename "$PROFILE") with PATH"
}

echo "Installing Alphabet Language ($PLATFORM $ARCH)..."
echo ""

# Try pre-built release
LATEST=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" 2>/dev/null \
    | grep -o '"tag_name": *"[^"]*"' | cut -d'"' -f4 || true)

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
        echo "Open a new terminal, or in this terminal run:"
        echo "  hash -r && source ~/.bashrc"
        echo "  alphabet --repl"
        echo ""
        echo "Update:  curl -fsSL https://raw.githubusercontent.com/$REPO/main/install.sh | sh"
        echo "Remove:  curl -fsSL https://raw.githubusercontent.com/$REPO/main/install.sh | sh -s -- --uninstall"
        exit 0
    fi
fi

# Fallback: build from source
echo "No pre-built release available. Building from source..."
echo ""

# Check dependencies
MISSING=""
for cmd in git cmake make; do
    command -v "$cmd" >/dev/null 2>&1 || MISSING="$MISSING $cmd"
done

# Check for C++ compiler (g++ or clang++)
if ! command -v g++ >/dev/null 2>&1 && ! command -v clang++ >/dev/null 2>&1; then
    MISSING="$MISSING g++ (or clang++)"
fi

if [ -n "$MISSING" ]; then
    echo "Missing dependencies:$MISSING"
    if [ "$OS" = "Darwin" ]; then
        echo "Install: xcode-select --install && brew install cmake"
    else
        echo "Install: sudo apt install cmake g++ make"
    fi
    exit 1
fi

cd "$TMPDIR"
git clone --depth 1 "https://github.com/$REPO.git" alphabet
cd alphabet

echo "Compiling..."
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build -DCMAKE_MESSAGE_LOG_LEVEL=ERROR

# Get core count (works on Linux and macOS)
JOBS=$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 2)
cmake --build build -j"$JOBS"

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
echo "Open a new terminal, or in this terminal run:"
echo "  hash -r && source ~/.bashrc"
echo "  alphabet --repl"
