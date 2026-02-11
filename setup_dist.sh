#!/bin/bash
# Alphabet Professional Release Script
# Developed by Fraol Teshome

echo "------------------------------------------------"
echo "Preparing Production Binaries (Hidden Logic)"
echo "------------------------------------------------"

# Ensure Nuitka is installed
pip install -U nuitka --break-system-packages

# Function to compile
compile_alphabet() {
    local OS=$1
    echo "Compiling for $OS..."
    python3 -m nuitka \
        --standalone \
        --onefile \
        --follow-imports \
        --remove-output \
        --output-filename=alphabet \
        alphabet.py
}

# 1. Compile for the current machine
compile_alphabet "Current OS"

echo ""
echo "------------------------------------------------"
echo "HOW TO SHARE ON GITHUB (WITHOUT SHOWING CODE):"
echo "1. Create a Public GitHub Repo."
echo "2. Upload ONLY the 'docs' and 'examples' folders."
echo "3. Go to the 'Releases' tab on GitHub."
echo "4. Upload the compiled 'alphabet' file there."
echo "5. Tell users: 'Download the binary from the Release tab'."
echo "------------------------------------------------"
