# Phase 1 Audit: VS Code Extension for Alphabet Language v2.3.5

**Goal:** `alphabet-vscode-v1` — Fully-featured VS Code extension for Alphabet Language v2.3.5 with zero-install WASM LSP
**Audit Date:** 2026-06-21
**Audited:** `editors/vscode-alphabet/` v0.1.0 + WASM build artifacts

---

## TL;DR — Where We Are

The extension **exists and works for users who already have `alphabet` on PATH**, but it does **not** meet the success criteria in the goal:

- ❌ **Zero-install** — bundles only `alphabet-linux-x64`; no Win/macOS/ARM binaries, no WASM fallback in extension
- ⚠️ **Feature parity ~50%** — syntax, LSP client, NL-to-Code, snippets, walkthrough, formatter-registered. Missing: debug adapter, REPL, test explorer, multi-platform binaries, semantic tokens, code actions, inlay hints
- ⚠️ **Security ~30%** — multiple unmitigated attack surfaces (see §4)
- ⚠️ **Marketplace readiness ~60%** — placeholder publisher `fraolteshome`, missing LICENSE double (LICENSE + LICENSE.md both 1087 bytes), icon-mismatch risk

**Verdict:** A working v0.1.0 exists. Real "fully featured + zero-install" needs ~6-10 milestones, not a single PR.

---

## 1. Extension Inventory

### What exists (`editors/vscode-alphabet/`)

| Asset | Status | Notes |
|---|---|---|
| `package.json` | ✅ v0.1.0, publisher `fraolteshome` | Categories include Linters, Formatters, Snippets, Debuggers |
| `syntaxes/alphabet.tmLanguage.json` | ✅ | TextMate grammar |
| `snippets/alphabet.json` | ✅ | |
| `language-configuration.json` | ✅ | firstLine `#alphabet<[a-z]{2}>` |
| `src/extension.ts` | ✅ 570 LOC | LSP client + commands + walkthrough |
| `src/nl-to-code.ts` | ✅ 256 LOC | NL-to-Code webview panel |
| `server/bin/alphabet-linux-x64` | ⚠️ bundled native binary | Linux x64 ONLY — Win/macOS/ARM users fail |
| `dist/extension.js` | ✅ esbuild bundle | |
| `alphabet-0.1.0.vsix` | ✅ 660 KB packaged | |
| `templates/` | ✅ | |
| `resources/` | ✅ icon + view-icon + walkthrough | |
| `scripts/` | ✅ | |
| **WASM runtime** | ❌ NOT bundled | `build-wasm/alphabet.wasm` (488K) + `alphabet.js` (16K) exist in repo root but extension doesn't load them |
| **Debug adapter** | ❌ absent | No `DebugAdapterDescriptorFactory`, no `package.json#contributes.debuggers` |
| **REPL** | ❌ absent | No terminal-based interactive runner |
| **Test Explorer** | ❌ absent | No `alphabet.test` integration |

### Commands implemented (12)

`alphabet.run`, `alphabet.lint`, `alphabet.compile`, `alphabet.buildWasm`, `alphabet.nlToCode`, `alphabet.nlToCode.focus`, `alphabet.restartServer`, `alphabet.showOutput`, `alphabet.openDocs`, `alphabet.newFile`, `alphabet.openWalkthrough`

### LSP capability coverage (client-side; backend must support)

| LSP method | Client registers | Backend (alphabet binary) confirms |
|---|---|---|
| `completion` | ✅ via client | ⚠️ need to verify against binary v2.3.5 |
| `hover` | ✅ with Markdown middleware | ⚠️ need to verify |
| `gotoDefinition` | ✅ via client | ⚠️ need to verify |
| `diagnostics` | ✅ via client | ⚠️ need to verify |
| `signatureHelp` | ❌ not exercised | ❓ unknown |
| `documentHighlight` | ❌ | ❓ |
| `documentLink` | ❌ | ❓ |
| `codeAction` | ❌ | ❓ |
| `rename` | ❌ | ❓ |
| `documentFormatting` | ✅ registers but only delegates to server | ⚠️ binary's `alphabet fmt` exists |
| `executeCommand` | ❌ | ❓ |
| `semanticTokens` | ❌ | ❓ |
| `inlayHint` | ❌ | ❓ |

---

## 2. v2.3.5 Feature Surface vs Extension Coverage

| v2.3.5 feature | In extension | Gap |
|---|---|---|
| 81 builtins (`z.o`, `z.i`, `z.sqrt`, …) | ⚠️ partial — depends on LSP backend coverage | Need to grep server-side completion list |
| 45 opcodes | n/a (internal) | — |
| 15 type IDs | ⚠️ partial — hover docs should cover | Need verification |
| 5 languages (en/am/de/es/fr) | ✅ via `firstLine` regex + grammar | |
| 26 keywords per language | ⚠️ grammar may not cover all variants | Need to inspect TextMate grammar |
| Multi-language code in same file | ❓ grammar likely per-language only | — |
| 25 stdlib modules | ⚠️ completion-only via LSP | — |
| NL-to-Code | ✅ webview panel | |
| Voice input | ❌ | Skill notes: voice server is Python + REPL pipe; not feasible in extension host |
| Threading builtins (`z.thread`, `z.join`, …) | ⚠️ completion only | LSP should expose |
| HTTP builtins (`z.http_get`, `z.http_post`) | ⚠️ completion only | — |
| JSON builtins (`z.json_parse`, `z.json_stringify`) | ⚠️ completion only | — |
| WASM target (`alphabet build-wasm`) | ⚠️ `alphabet.buildWasm` command shells to binary; no bundled WASM | Hard blocker for zero-install |
| LSP server (`alphabet --lsp`) | ✅ via stdio | |
| Debugger (`alphabet --debug`) | ❌ | |
| REPL | ❌ | |
| Watch mode (`alphabet watch`) | ❌ | |
| Profiler (`--profile`) | ❌ | |
| Linter (`alphabet lint`) | ✅ command shells to binary | |
| Formatter (`alphabet fmt`) | ✅ registered as LSP formatter | |
| Dependency mgmt (`alphabet init/install/publish`) | ❌ no project init command | — |

---

## 3. The Zero-Install Blocker — Current Architecture

`src/extension.ts:88-95`:
```ts
const run: Executable = {
  command: binaryPath,
  args: ['--lsp'],
  transport: TransportKind.stdio,
  options: {
    env: { ...process.env, ALPHABET_LSP_CLIENT: 'vscode' },
  },
};
```

`pickBundledBinary()` (lines 102-115) only resolves `alphabet-linux-x64`. Other platforms fall through to `findOnPath('alphabet')`.

**What's needed for zero-install:**
1. Bundle WASM + JS glue into extension `dist/` or `server/bin/`
2. Spawn a Node.js worker (or use `WebAssembly.instantiate` directly) that loads `alphabet.wasm` and exposes an LSP-over-stdio bridge
3. Bundle platform-specific fallback binaries (or drop platform support and rely on WASM only)
4. Update `resolveServerBinary()` to prefer WASM before binary
5. **Critical:** the current C++ LSP server (alphabet binary `--lsp` mode) is **separate from the WASM build** (alphabet.wasm runs the VM, not the LSP). The WASM build is a program-runner; the LSP server is C++-only. Need to either (a) port the LSP server to run inside WASM and bridge stdio, or (b) build a JS LSP frontend that compiles + analyzes using a JS port of the parser.

The skill notes (`alphabet-language-debugging` pitfall #28): *"JS interpreter CANNOT match native binary: Missing lambdas, threading, FFI, type enforcement, const enforcement, ternary, super(), constant folding. WASM is the ONLY way for full v2.3.5 in browser."*

So the path is: **bundle WASM + build an LSP-in-WASM bridge** (work not yet done in the repo).

---

## 4. Security Audit — Findings

### 🔴 HIGH

**S1. Arbitrary binary execution via `alphabet.lsp.path` setting**
- `src/extension.ts:117-145` accepts user-configured `alphabet.lsp.path` without validation
- A malicious repo (or settings.json under attacker control) can point to any executable
- The binary is spawned with full environment + stdio
- **Fix:** Validate the path resolves to a file inside `server/bin/` OR is on a trusted allowlist; check SHA-256 of bundled binary matches a known value; warn user when using a non-bundled binary

**S2. Shell injection via `alphabet.run.command` setting**
- The README/setting documents `${file}` placeholder substitution, but the implementation in `src/extension.ts` uses `execFileSync` (good) — need to verify the actual substitution function escapes properly
- A user setting `alphabet.run.command = "rm -rf $HOME && alphabet run ${file}"` would NOT be exec-injected (execFileSync is array-based), but a malicious workspace could redefine the user's settings
- **Fix:** When `alphabet.lsp.path` or `alphabet.run.command` come from workspace scope, prompt user with confirmation; or restrict to user scope only

**S3. NL-to-Code prompt injection vector**
- `src/nl-to-code.ts` webview accepts free-form natural language and produces code
- No evidence of input sanitization, no max length, no rate limit
- If NL-to-Code ever calls an external LLM, this is a prompt injection surface
- If it uses a rule-based converter (skill notes suggest it does), lower risk — but no validation that generated code is sandboxed
- **Fix:** Cap input length, log generated code for review, sandbox execution, never auto-run NL→code output

### 🟡 MEDIUM

**S4. No telemetry opt-out enforcement**
- `package.json` declares `alphabet.telemetry.enabled` (default false)
- **Fix:** Verify no code paths send data when disabled; add explicit test

**S5. Dependencies**
- `devDependencies` and `dependencies` not in this view; need `npm audit` pass
- **Fix:** Run `npm audit --omit=dev` and `npm audit`; pin `vscode-languageclient` ^9.0.1

**S6. Untrusted workspace trust model**
- Extension activates on `workspaceContains:**/*.abc` — even untrusted workspaces trigger LSP start
- LSP server has full FS access (it's an executable on the user's machine)
- **Fix:** Use `workspace.isTrusted` check before activating heavy features; gate NL-to-Code behind trust

### 🟢 LOW

**S7. Output channel can leak binary stderr**
- Line 95: env passed through; if binary prints sensitive paths to stderr, they show in VS Code output
- **Fix:** Sanitize or collapse long lines; add "Copy" button with redaction

**S8. Bundled binary has no signature verification**
- The bundled `alphabet-linux-x64` is a raw ELF; if the extension package is repacked by a malicious actor, the binary runs as-is
- **Fix:** Add SHA-256 manifest + vsce signing

---

## 5. Critical Issues to Fix Before Any Feature Work

Before adding more features, these need to be resolved:

1. **Multi-platform binary bundling** OR **WASM-bundled LSP** (zero-install success criterion)
2. **Path allowlist validation** (S1)
3. **Workspace trust gate** (S6)
4. **Remove placeholder publisher** — `fraolteshome` should be replaced or documented
5. **Deduplicate LICENSE/LICENSE.md** — both 1087 bytes

---

## 6. Recommended Phase 2 Plan (proposed, not executed)

If you approve Phase 2, here's the proposed milestone breakdown:

| # | Milestone | Deliverable |
|---|---|---|
| M1 | Multi-platform binary bundle | `server/bin/alphabet-{linux,darwin,win32}-{x64,arm64}` for all 6 platforms |
| M2 | WASM-bundled LSP | Extension loads `alphabet.wasm` via Worker, bridges stdio↔LSP |
| M3 | Security hardening | S1-S6 fixes; security review sign-off |
| M4 | Full LSP capability | signatureHelp, codeAction, rename, semanticTokens, inlayHints (verify backend coverage first) |
| M5 | Debug adapter | `alphabet --debug` → DAP over stdio |
| M6 | REPL in terminal | Side-panel REPL with trace mode |
| M7 | Test Explorer | `alphabet test` integration |
| M8 | Marketplace publish | Publisher set, icon set, CHANGELOG, screenshots, vsce preflight clean |

Estimated effort: M1 = 1 session, M2 = 3-5 sessions (largest), M3 = 1 session, M4-M7 = 1-2 sessions each, M8 = 0.5 session. Total: ~10-15 sessions of focused work.

---

## 7. Open Questions for You

1. **WASM-first or binary-first?** WASM = true zero-install, ~488KB overhead. Binary = faster cold start, full feature parity guaranteed.
2. **Drop native binary entirely** in favor of WASM? Or keep both as selectable transports?
3. **Debug adapter priority** — many users won't use Alphabet's debugger; is M5 a hard requirement for "fully featured"?
4. **Publisher identity** — `fraolteshome` looks like a placeholder. What's the real publisher name for marketplace?
5. **License** — MIT confirmed? Both files are 1087 bytes which is suspicious.
