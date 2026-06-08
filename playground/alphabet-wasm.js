/**
 * Alphabet Language — WebAssembly JavaScript Wrapper
 * Usage: const alpha = new Alphabet(); await alpha.init(); alpha.run("code");
 */
class Alphabet {
  constructor() {
    this.module = null;
    this.ready = false;
  }

  async init() {
    if (typeof Module !== 'undefined') {
      this.module = Module;
    } else {
      throw new Error('Alphabet WASM module not loaded. Include alphabet.js first.');
    }

    return new Promise((resolve) => {
      if (this.module.calledRun) {
        this.ready = true;
        resolve();
      } else {
        this.module.onRuntimeInitialized = () => {
          this.ready = true;
          resolve();
        };
      }
    });
  }

  run(code) {
    if (!this.ready) throw new Error('Alphabet not initialized. Call init() first.');
    try {
      const result = this.module.ccall('alphabet_run', 'string', ['string'], [code]);
      return { success: true, output: result };
    } catch (e) {
      return { success: false, error: e.message };
    }
  }

  eval(expression) {
    if (!this.ready) throw new Error('Alphabet not initialized. Call init() first.');
    try {
      const result = this.module.ccall('alphabet_eval', 'string', ['string'], [expression]);
      return { success: true, output: result };
    } catch (e) {
      return { success: false, error: e.message };
    }
  }

  version() {
    if (!this.ready) throw new Error('Alphabet not initialized.');
    return this.module.ccall('alphabet_version', 'string', [], []);
  }
}

// Export for different module systems
if (typeof module !== 'undefined' && module.exports) {
  module.exports = Alphabet;
}
if (typeof window !== 'undefined') {
  window.Alphabet = Alphabet;
}
