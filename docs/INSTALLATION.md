# Installing Alphabet

**Complete installation guide for all platforms.**

---

## Quick Install

Download from [GitHub Actions](https://github.com/fraol163/alphabet/actions):
- **Linux:** `alphabet-linux`
- **macOS:** `alphabet-macos`
- **Windows:** `alphabet-windows.exe`

---

## Build from Source

### Linux (Ubuntu/Debian)

```bash
sudo apt install cmake g++ make
git clone https://github.com/fraol163/alphabet.git
cd alphabet/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

### macOS

```bash
xcode-select --install
brew install cmake
git clone https://github.com/fraol163/alphabet.git
cd alphabet/build
cmake -DCMAKE_BUILD_TYPE=Release ..
make -j$(nproc)
sudo make install
```

### Windows

```powershell
# Install Visual Studio 2019+ with C++
# Install CMake from https://cmake.org/

git clone https://github.com/fraol163/alphabet.git
cd alphabet/build
cmake -DCMAKE_BUILD_TYPE=Release ..
cmake --build . --config Release
```

---

## Verify

```bash
alphabet --version
```

**Expected:**
```
Alphabet 2.0.0 (Native C++)
Developer: Fraol Teshome (fraolteshome444@gmail.com)
```

---

**Next:** [Getting Started](GETTING_STARTED.md)
