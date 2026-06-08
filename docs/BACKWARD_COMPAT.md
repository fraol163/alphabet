# Backward Compatibility Policy

## Principles

1. **Stability:** Users' code should continue to work across minor versions
2. **Predictability:** Breaking changes only in major versions
3. **Communication:** Deprecation warnings before removal
4. **Migration:** Clear upgrade paths for breaking changes

## Versioning Rules

### Minor Version (2.3.x → 2.4.0)
**May add:**
- New keywords
- New builtins
- New stdlib modules
- New CLI commands
- New language features

**Will NOT break:**
- Existing keywords
- Existing builtins
- Existing syntax
- Existing stdlib modules

### Major Version (2.x → 3.0.0)
**May break:**
- Keyword changes
- Syntax changes
- Builtin signature changes
- Stdlib API changes

**Will provide:**
- 6-month deprecation warning
- Migration guide
- Automated migration tool (if possible)

## Deprecation Process

### Step 1: Warning (v2.4.0)
```
Warning: 'z.old_function()' is deprecated. Use 'z.new_function()' instead.
  Will be removed in v3.0.0.
```

### Step 2: Error (v2.5.0)
```
Error: 'z.old_function()' is removed. Use 'z.new_function()' instead.
  See migration guide: docs/MIGRATION.md
```

### Step 3: Removal (v3.0.0)
Function is completely removed.

## Current Deprecations

None currently deprecated.

## Stability Guarantees

### Guaranteed Stable (no breaking changes without major version)
- All 19 keywords (i, e, l, b, k, r, c, m, n, v, p, s, a, j, t, h, q, x, z)
- `#alphabet<lang>` header format
- Bytecode format (ALPH header)
- REPL commands
- CLI flags
- z. builtin function signatures

### May Change (minor version)
- New opcodes added
- New builtins added
- New keywords added
- Stdlib modules expanded
- Error messages improved

### Experimental (may change anytime)
- `--profile` flag output format
- `alphabet bench` benchmark numbers
- Embedding API (`alphabet_embed.h`)
- Bytecode cache format (`.abcc`)
- Voice input feature

## Testing

Every release passes:
- 91+ unit tests
- 30 golden file tests
- 24 examples
- CI/CD on Linux + macOS

## Rollback Policy

If a minor version introduces a regression:
1. Patch release within 48 hours
2. Hotfix branch from last stable
3. Emergency release if critical
