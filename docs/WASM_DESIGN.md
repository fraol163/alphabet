# Alphabet Language — WebAssembly Target Design

## Overview

Compile Alphabet bytecode to WebAssembly so it can run in browsers.

## Architecture

```
┌─────────────────────────────────────────────┐
│  Browser                                    │
│  ┌─────────────────────────────────────────┐│
│  │  JavaScript Wrapper                     ││
│  │  • Load .abc files                      ││
│  │  • Call Alphabet API                    ││
│  │  • Capture output                       ││
│  └────────────────┬────────────────────────┘│
│                   │                          │
│  ┌────────────────▼────────────────────────┐│
│  │  WebAssembly Module                     ││
│  │  • Alphabet VM (compiled from C++)      ││
│  │  • Lexer, Parser, Compiler, VM          ││
│  │  • All 82 builtins                     ││
│  └─────────────────────────────────────────┘│
└─────────────────────────────────────────────┘
```

## Implementation Plan

### Step 1: Emscripten Build
```bash
# Install Emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk && ./emsdk install latest && ./emsdk activate latest
source ./emsdk_env.sh

# Build Alphabet for WASM
emcmake cmake -S . -B build-wasm
cmake --build build-wasm
```

### Step 2: JavaScript Wrapper
```javascript
// alphabet-wasm.js
class Alphabet {
  constructor() {
    this.module = null;
  }

  async init() {
    this.module = await AlphabetModule();
  }

  run(code) {
    return this.module._alphabet_run(code);
  }

  eval(expression) {
    return this.module._alphabet_eval(expression);
  }
}
```

### Step 3: Browser REPL
```html
<!DOCTYPE html>
<html>
<head>
  <title>Alphabet Playground</title>
  <script src="alphabet-wasm.js"></script>
</head>
<body>
  <textarea id="code">#alphabet&lt;en&gt;
z.o("Hello from browser!")</textarea>
  <button onclick="runCode()">Run</button>
  <pre id="output"></pre>

  <script>
    const alpha = new Alphabet();
    alpha.init();

    function runCode() {
      const code = document.getElementById('code').value;
      const result = alpha.run(code);
      document.getElementById('output').textContent = result;
    }
  </script>
</body>
</html>
```

## Challenges

1. **File I/O:** No filesystem in browser — use in-memory virtual FS
2. **FFI:** No native FFI in browser — disable or use JS bridge
3. **Audio:** No microphone in WASM — use Web Audio API
4. **Size:** WASM binary ~2-5MB — acceptable for web

## Dependencies

- Emscripten SDK
- CMake (with Emscripten toolchain)
- No additional libraries needed

## Timeline

- **Week 1:** Emscripten build, basic WASM module
- **Week 2:** JavaScript wrapper, browser REPL
- **Week 3:** Testing, optimization, documentation

## Status

- [x] Design document
- [ ] Emscripten build
- [ ] JavaScript wrapper
- [ ] Browser REPL
- [ ] Testing
- [ ] Documentation
