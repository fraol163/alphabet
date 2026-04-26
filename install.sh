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

command_exists() { command -v "$1" >/dev/null 2>&1; }

# Uninstall support
if [ "${1:-}" = "--uninstall" ]; then
    rm -f "$INSTALL_DIR/alphabet" "$INSTALL_DIR/alphabet.exe"
    hash -r 2>/dev/null || true
    success "Uninstalled alphabet from $INSTALL_DIR"
    exit 0
fi

detect_os() {
    case "$(uname -s)" in
        Linux*)  echo "linux" ;;
        Darwin*) echo "macos" ;;
        MINGW*|MSYS*|CYGWIN*) echo "windows" ;;
        *) error "Unsupported OS: $(uname -s)" ;;
    esac
}

detect_arch() {
    case "$(uname -m)" in
        x86_64|amd64) echo "amd64" ;;
        arm64|aarch64) echo "arm64" ;;
        *) error "Unsupported architecture: $(uname -m)" ;;
    esac
}

get_latest_version() {
    curl -fsSL "https://api.github.com/repos/$REPO/releases/latest" \
        | grep -o '"tag_name": *"[^"]*"' \
        | head -1 \
        | sed -E 's/.*"v?([^"]+)".*/\1/'
}

install_raw_binary() {
    asset_name="$1"
    ext="$2"
    url="https://github.com/$REPO/releases/download/v$LATEST/$asset_name"

    info "Trying $asset_name"

    TMPDIR="$(mktemp -d)"
    TMP_FILE="$TMPDIR/$asset_name"
    trap 'rm -rf "$TMPDIR"' EXIT

    if curl -fL --retry 3 --connect-timeout 10 -o "$TMP_FILE" "$url" >/dev/null 2>&1; then
        mkdir -p "$INSTALL_DIR"
        if [ "$OS" = "windows" ]; then
            mv "$TMP_FILE" "$INSTALL_DIR/alphabet.exe"
            cat > "$INSTALL_DIR/alphabet" <<'EOF'
#!/bin/sh
DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
exec "$DIR/alphabet.exe" "$@"
EOF
            chmod 755 "$INSTALL_DIR/alphabet" 2>/dev/null || true
            hash -r 2>/dev/null || true
            success "Installed to $INSTALL_DIR/alphabet.exe"
            return 0
        fi

        chmod +x "$TMP_FILE"
        mv "$TMP_FILE" "$INSTALL_DIR/alphabet"
        hash -r 2>/dev/null || true
        success "Installed to $INSTALL_DIR/alphabet"
        return 0
    fi

    return 1
}

install_archive() {
    archive_name="$1"
    url="https://github.com/$REPO/releases/download/v$LATEST/$archive_name"

    info "Trying $archive_name"

    TMPDIR="$(mktemp -d)"
    trap 'rm -rf "$TMPDIR"' EXIT
    TMP_FILE="$TMPDIR/$archive_name"
    EXTRACT_DIR="$TMPDIR/extract"
    mkdir -p "$EXTRACT_DIR"

    if ! curl -fL --retry 3 --connect-timeout 10 -o "$TMP_FILE" "$url" >/dev/null 2>&1; then
        return 1
    fi

    if ! tar -xf "$TMP_FILE" -C "$EXTRACT_DIR" >/dev/null 2>&1; then
        return 1
    fi

    BIN_SRC=""
    if [ -f "$EXTRACT_DIR/alphabet" ]; then
        BIN_SRC="$EXTRACT_DIR/alphabet"
    elif [ -f "$EXTRACT_DIR/alphabet.exe" ]; then
        BIN_SRC="$EXTRACT_DIR/alphabet.exe"
    else
        BIN_SRC="$(find "$EXTRACT_DIR" -type f \( -name alphabet -o -name alphabet.exe \) | head -1 || true)"
    fi

    if [ -z "$BIN_SRC" ] || [ ! -f "$BIN_SRC" ]; then
        return 1
    fi

    mkdir -p "$INSTALL_DIR"
    if [ "$OS" = "windows" ]; then
        mv "$BIN_SRC" "$INSTALL_DIR/alphabet.exe"
        cat > "$INSTALL_DIR/alphabet" <<'EOF'
#!/bin/sh
DIR="$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)"
exec "$DIR/alphabet.exe" "$@"
EOF
        chmod 755 "$INSTALL_DIR/alphabet" 2>/dev/null || true
        hash -r 2>/dev/null || true
        success "Installed to $INSTALL_DIR/alphabet.exe"
    else
        mv "$BIN_SRC" "$INSTALL_DIR/alphabet"
        chmod +x "$INSTALL_DIR/alphabet"
        hash -r 2>/dev/null || true
        success "Installed to $INSTALL_DIR/alphabet"
    fi
    return 0
}

main() {
    echo ""
    printf "${CYAN}   ╔══════════════════════════════╗${NC}\n"
    printf "${CYAN}   ║   Alphabet Language Installer ║${NC}\n"
    printf "${CYAN}   ╚══════════════════════════════╝${NC}\n"
    echo ""

    if ! command_exists curl; then
        error "curl is required but not installed."
    fi

    if ! command_exists tar; then
        error "tar is required but not installed."
    fi

    OS=$(detect_os)
    ARCH=$(detect_arch)
    info "Detected: $OS-$ARCH"

    if command_exists alphabet; then
        CURRENT=$(alphabet --version 2>/dev/null | head -1 | grep -oE '[0-9]+\.[0-9]+\.[0-9]+' || echo "unknown")
        info "Found existing Alphabet v$CURRENT"
    else
        CURRENT="none"
    fi

    info "Fetching latest version..."
    LATEST=$(get_latest_version)
    if [ -z "$LATEST" ]; then
        error "Could not fetch latest version from GitHub"
    fi
    info "Latest version: v$LATEST"

    case "$OS-$ARCH" in
        linux-amd64)
            RAW_ASSET="alphabet-linux-amd64"
            LEGACY_ARCHIVE="alphabet-${LATEST}-Linux-x86_64.tar.gz"
            ;;
        macos-amd64)
            RAW_ASSET="alphabet-macos-amd64"
            LEGACY_ARCHIVE="alphabet-${LATEST}-Darwin-x86_64.tar.gz"
            ;;
        macos-arm64)
            RAW_ASSET="alphabet-macos-arm64"
            LEGACY_ARCHIVE="alphabet-${LATEST}-Darwin-arm64.tar.gz"
            ;;
        windows-amd64)
            RAW_ASSET="alphabet-windows-amd64.exe"
            LEGACY_ARCHIVE="alphabet-${LATEST}-Windows-AMD64.zip"
            ;;
        *)
            error "Unsupported platform combination: $OS-$ARCH"
            ;;
    esac

    if [ "$CURRENT" = "$LATEST" ]; then
        success "Already up to date!"
        exit 0
    fi

    if install_raw_binary "$RAW_ASSET"; then
        :
    elif install_archive "$LEGACY_ARCHIVE"; then
        :
    else
        error "Could not download a compatible release asset for $OS-$ARCH"
    fi

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

    echo ""
    if [ "$OS" = "windows" ]; then
        info "Run 'alphabet.exe --help' or 'alphabet --help' to get started."
        success "Alphabet v$LATEST installed successfully!"
    else
        if command_exists alphabet; then
            success "Alphabet v$LATEST installed successfully!"
            echo ""
            alphabet --version
        else
            success "Alphabet v$LATEST installed successfully!"
            echo ""
            info "Run '$INSTALL_DIR/alphabet --help' to get started."
        fi
    fi
}

main "$@"
