# Alphabet Language — Remaining Items Documentation

## Code Items (3 remaining)

### 1. WebAssembly Target (W50)
**Status:** Not implemented  
**Complexity:** 1 week  
**Description:** Compile Alphabet bytecode to WebAssembly so it can run in browsers.  
**Approach:** Use Emscripten to compile the VM to WASM. Create a JS wrapper that loads .abc files and executes them in the browser.  
**Dependencies:** Emscripten SDK, JavaScript runtime  

### 2. Dependency Management (W121)
**Status:** Partially implemented  
**Complexity:** 1 week  
**Description:** Full dependency resolution with lock files and version pinning.  
**Current:** alphabet.toml parser exists, can declare dependencies.  
**Missing:** Lock file, version resolution, remote package fetching.  
**Approach:** Extend ProjectManager to support `alphabet install` command.  

### 3. Threading/Concurrency (W86)
**Status:** Not implemented  
**Complexity:** 2 weeks  
**Description:** Multi-threaded execution with thread-safe data structures.  
**Approach:** Add `thread` keyword, create worker threads, implement mutex/lock primitives.  
**Challenge:** Requires significant VM changes for thread safety.  

## Non-Code Items (42 remaining)

### Ecosystem (15 items)
- **Homebrew formula** — Create homebrew-alphabet tap
- **AUR package** — PKGBUILD for Arch Linux
- **Snap package** — snapcraft.yaml for Ubuntu
- **Docker Hub** — Publish official Docker image
- **Package manager** — `alphabet install` for dependencies
- **Workspace support** — Multi-file project builds
- **Stdlib versioning** — Version stdlib modules
- **Stdlib backward compat** — Deprecation warnings
- **Language spec** — Formal language specification
- **Public roadmap** — GitHub Projects board
- **Versioning policy** — Semver documentation (DONE)
- **Backward compat policy** — Breaking change rules
- **Nightly channel** — Automated nightly builds (DONE)
- **Beta releases** — Pre-release channel
- **Contributing guide** — How to contribute (DONE)

### Product (15 items)
- **Business model** — Sustainability plan
- **Growth strategy** — Launch and adoption plan
- **User research** — Beginner usability testing
- **Competitive analysis** — vs Scratch, Python, Lua
- **Analytics/telemetry** — Usage tracking (opt-in)
- **Testimonials** — User success stories
- **Case studies** — Education adoption examples
- **University partnerships** — Academic adoption
- **Conference talks** — Present at tech conferences
- **Content marketing** — Blog posts, tutorials
- **Social media** — Twitter, YouTube presence
- **Documentation site** — alphabet-lang.org
- **Interactive playground** — Browser-based REPL
- **Video tutorials** — YouTube series
- **Book/course** — "Learn Programming with Alphabet"

### Philosophy (12 items)
- **Type system coherence** — Strengthen type checking
- **Keyword design review** — Evaluate single-letter approach
- **Error message quality** — Improve error suggestions
- **Performance benchmarks** — vs Python, Lua, JS
- **Memory safety audit** — Review raw pointer usage
- **API design review** — Consistency audit
- **Naming conventions** — Standardize naming
- **Code style guide** — Contributor style rules
- **Release process** — Automated release workflow
- **Changelog automation** — Auto-generate changelogs
- **Issue triage process** — Bug report workflow
- **Security policy** — Vulnerability reporting

## Implementation Priority

```
HIGH (do next):
  1. Homebrew formula
  2. AUR package
  3. Docker Hub publish
  4. Language spec (BNF grammar — DONE)

MEDIUM (do when possible):
  5. Dependency management (full)
  6. Workspace support
  7. Public roadmap
  8. Documentation site

LOW (requires resources):
  9. WebAssembly target
  10. Threading
  11. Business model
  12. Growth strategy
  13. User research
  14. University partnerships
  15. Conference talks
```
