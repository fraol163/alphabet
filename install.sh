#!/bin/sh
# Alphabet Language Installer
# Downloads pre-built release from GitHub, installs, and starts REPL
# Usage: curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh

set -e

REPO="fraol163/alphabet"
INSTALL_DIR="${ALPHABET_HOME:-/usr/local}"
BIN_DIR="$INSTALL_DIR/bin"
LIB_DIR="$INSTALL_DIR/share/alphabet"

# Handle uninstall
if [ "$1" = "--uninstall" ]; then
    echo "Uninstalling Alphabet Language..."
    sudo rm -f "$BIN_DIR/alphabet"
    sudo rm -rf "$LIB_DIR"
    echo "Alphabet uninstalled."
    exit 0
fi

# Detect platform
OS="$(uname -s)"
case "$OS" in
    Linux*)  PLATFORM="linux";  ARCHIVE="alphabet-linux-amd64.tar.gz" ;;
    Darwin*) PLATFORM="macos";  ARCHIVE="alphabet-macos-arm64.tar.gz" ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "Windows: download alphabet-windows-amd64.zip from"
        echo "  https://github.com/$REPO/releases/latest"
        exit 0
        ;;
    *) echo "Unsupported OS: $OS"; exit 1 ;;
esac

echo "Installing Alphabet Language ($PLATFORM)..."

# Get latest release tag
LATEST=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" | grep -o '"tag_name": *"[^"]*"' | cut -d'"' -f4)
if [ -z "$LATEST" ]; then
    echo "Error: could not fetch latest release"
    exit 1
fi
echo "  Version: $LATEST"

# Download
URL="https://github.com/$REPO/releases/download/$LATEST/$ARCHIVE"
TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

echo "  Downloading $ARCHIVE ..."
curl -fsSL "$URL" -o "$TMPDIR/alphabet.tar.gz"

# Extract
cd "$TMPDIR"
tar xzf alphabet.tar.gz

# Install
echo "  Installing to $BIN_DIR ..."
sudo mkdir -p "$BIN_DIR" "$LIB_DIR"
sudo cp alphabet "$BIN_DIR/alphabet"
sudo chmod +x "$BIN_DIR/alphabet"
[ -d stdlib ] && sudo cp -r stdlib/* "$LIB_DIR/"

echo ""
echo "Alphabet $LATEST installed!"
echo ""

# Start REPL
exec "$BIN_DIR/alphabet" --repl
