#!/bin/sh
# Alphabet Language Installer
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
    echo "Alphabet uninstalled from $INSTALL_DIR"
    exit 0
fi

# Detect OS and architecture
OS="$(uname -s)"
ARCH="$(uname -m)"

case "$OS" in
    Linux*)     PLATFORM="linux" ;;
    Darwin*)    PLATFORM="macos" ;;
    MINGW*|MSYS*|CYGWIN*)
        echo "Windows detected. Download from GitHub Releases:"
        echo "  https://github.com/$REPO/releases/latest"
        exit 0
        ;;
    *)          echo "Error: Unsupported OS: $OS"; exit 1 ;;
esac
echo "  Platform: $PLATFORM"
echo "  Install dir: $INSTALL_DIR"
echo ""

# Get latest release URL
LATEST=$(curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" | grep -o '"tag_name": *"[^"]*"' | cut -d'"' -f4)
if [ -z "$LATEST" ]; then
    echo "Error: Could not fetch latest release"
    exit 1
fi
echo "  Version: $LATEST"

# Download
DOWNLOAD_URL="https://github.com/$REPO/releases/download/$LATEST/alphabet-${PLATFORM}-amd64.tar.gz"
if [ "$PLATFORM" = "macos" ]; then
    DOWNLOAD_URL="https://github.com/$REPO/releases/download/$LATEST/alphabet-macos-arm64.tar.gz"
fi

TMPDIR=$(mktemp -d)
trap "rm -rf $TMPDIR" EXIT

echo "  Downloading from $DOWNLOAD_URL ..."
curl -fsSL "$DOWNLOAD_URL" -o "$TMPDIR/alphabet.tar.gz"

# Extract
cd "$TMPDIR"
tar xzf alphabet.tar.gz

# Install
echo "  Installing binary to $BIN_DIR ..."
sudo mkdir -p "$BIN_DIR" "$LIB_DIR"
sudo cp alphabet "$BIN_DIR/alphabet"
sudo chmod +x "$BIN_DIR/alphabet"

if [ -d stdlib ]; then
    echo "  Installing stdlib to $LIB_DIR ..."
    sudo cp -r stdlib/* "$LIB_DIR/"
fi

# Verify
if command -v alphabet >/dev/null 2>&1; then
    echo ""
    echo "Alphabet installed successfully!"
    alphabet --version
    echo ""
    echo "Usage:"
    echo "  alphabet program.abc     Run a program"
    echo "  alphabet --repl          Interactive mode"
    echo "  alphabet --help          Show help"
    echo ""
    echo "To uninstall: curl -fsSL https://raw.githubusercontent.com/$REPO/main/install.sh | sh -s -- --uninstall"
else
    echo ""
    echo "Installed to $BIN_DIR/alphabet"
    echo "Add $BIN_DIR to your PATH:"
    echo "  export PATH=\"$BIN_DIR:\$PATH\""
fi
