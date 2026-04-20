#!/bin/sh
# Alphabet Language Installer
# Usage: curl -sSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh

set -e

REPO="fraol163/alphabet"
INSTALL_DIR="${ALPHABET_DIR:-$HOME/.local/bin}"

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
CYAN='\033[0;36m'
NC='\033[0m'

info() { printf "${CYAN}▸${NC} %s\n" "$1"; }
success() { printf "${GREEN}✓${NC} %s\n" "$1"; }
error() { printf "${RED}✗${NC} %s\n" "$1"; exit 1; }

# Uninstall support
if [ "$1" = "--uninstall" ]; then
    rm -f "$INSTALL_DIR/alphabet"
    hash -r 2>/dev/null || true
    success "Uninstalled alphabet from $INSTALL_DIR"
    exit 0
fi

# Detect OS
detect_os() {
    case "$(uname -s)" in
        Linux*)  echo "linux" ;;
        Darwin*) echo "macos" ;;
        *)       error "Unsupported OS: $(uname -s)" ;;
    esac
}

# Detect architecture
detect_arch() {
    case "$(uname -m)" in
        x86_64|amd64)  echo "amd64" ;;
        aarch64|arm64) echo "arm64" ;;
        armv7l)        echo "arm" ;;
        *)             error "Unsupported architecture: $(uname -m)" ;;
    esac
}

# Get latest version from GitHub
get_latest_version() {
    local version
    version=$(curl -s "https://api.github.com/repos/$REPO/releases/latest" \
        | grep '"tag_name"' \
        | sed -E 's/.*"v?([^"]+)".*/\1/')
    
    if [ -z "$version" ]; then
        error "Could not fetch latest version. Check your internet connection."
    fi
    echo "$version"
}

# Check if command exists
command_exists() {
    command -v "$1" >/dev/null 2>&1
}

# Main
main() {
    echo ""
    printf "${CYAN}   ╔══════════════════════════════╗${NC}\n"
    printf "${CYAN}   ║   Alphabet Language Installer ║${NC}\n"
    printf "${CYAN}   ╚══════════════════════════════╝${NC}\n"
    echo ""

    # Check dependencies
    if ! command_exists curl; then
        error "curl is required but not installed."
    fi

    OS=$(detect_os)
    ARCH=$(detect_arch)
    info "Detected: $OS-$ARCH"

    # Check for existing installation
    if command_exists alphabet; then
        CURRENT=$(alphabet --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' || echo "unknown")
        info "Found existing Alphabet v$CURRENT"
    else
        CURRENT="none"
    fi

    # Get latest version
    info "Fetching latest version..."
    LATEST=$(get_latest_version)
    info "Latest version: v$LATEST"

    # Compare
    if [ "$CURRENT" = "$LATEST" ]; then
        success "Already up to date!"
        exit 0
    fi

    # Build download URL
    BINARY_NAME="alphabet-${OS}-${ARCH}"
    DOWNLOAD_URL="https://github.com/$REPO/releases/download/v${LATEST}/${BINARY_NAME}"
    
    info "Downloading from: $DOWNLOAD_URL"

    # Create install directory
    mkdir -p "$INSTALL_DIR"

    # Download
    TMP_FILE=$(mktemp)
    trap 'rm -f "$TMP_FILE"' EXIT
    if ! curl -L -o "$TMP_FILE" "$DOWNLOAD_URL" 2>/dev/null; then
        rm -f "$TMP_FILE"
        error "Download failed. Check: $DOWNLOAD_URL"
    fi

    # Install
    chmod +x "$TMP_FILE"
    mv "$TMP_FILE" "$INSTALL_DIR/alphabet"
    hash -r 2>/dev/null || true
    success "Installed to $INSTALL_DIR/alphabet"

    # Check PATH
    case ":$PATH:" in
        *":$INSTALL_DIR:"*)
            ;;
        *)
            echo ""
            info "Add $INSTALL_DIR to your PATH:"
            echo ""
            echo "  echo 'export PATH=\"$INSTALL_DIR:\$PATH\"' >> ~/.bashrc"
            echo "  source ~/.bashrc"
            echo ""
            ;;
    esac

    # Verify
    if command_exists alphabet; then
        success "Alphabet v$LATEST installed successfully!"
        echo ""
        alphabet --version
        echo ""
        info "Run 'alphabet --help' to get started."
    else
        info "Installed! Run: $INSTALL_DIR/alphabet --help"
    fi
}

main "$@"
