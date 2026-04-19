# Installing Alphabet

**Complete installation guide for all platforms.**

---

## One-Line Install (Linux/macOS)

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

This automatically:
- Downloads the pre-built binary from GitHub Releases
- Falls back to building from source if no release exists
- Installs to `/usr/local/bin/alphabet`
- Starts the REPL immediately

---

## Update

Same command as install:

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

---

## Uninstall

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh -s -- --uninstall
```

---

## Manual Download

Download from [GitHub Releases](https://github.com/fraol163/alphabet/releases/latest):

- **Linux:** `alphabet-linux-amd64.tar.gz`
- **macOS:** `alphabet-macos-arm64.tar.gz`
- **Windows:** `alphabet-windows-amd64.zip`

---

## Build from Source

### Linux (Ubuntu/Debian)

```bash
sudo apt install cmake g++ make
git clone https://github.com/fraol163/alphabet.git
cd alphabet
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build -j$(nproc)
sudo cmake --install build
```

### macOS

```bash
xcode-select --install
brew install cmake
git clone https://github.com/fraol163/alphabet.git
cd alphabet
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build -j$(sysctl -n hw.ncpu)
sudo cmake --install build
```

### Windows

```powershell
# Install Visual Studio 2019+ with C++
# Install CMake from https://cmake.org/

git clone https://github.com/fraol163/alphabet.git
cd alphabet
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build --config Release
```

---

## Environment Variables

| Variable | Description |
|----------|-------------|
| `ALPHABET_HOME` | Custom install directory (default: `/usr/local`) |
| `ALPHABET_PATH` | Colon-separated directories to search for imports |

---

## Verify

```bash
alphabet --version
```

**Expected:**
```
Alphabet 2.1.0 (Native C++)
Developer: Fraol Teshome (fraolteshome444@gmail.com)
Compiled with C++17
```

---

**Next:** [Getting Started](GETTING_STARTED.md)
