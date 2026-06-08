# Security Policy

## Supported Versions

| Version | Supported          |
| ------- | ------------------ |
| 2.3.x   | ✅ Yes             |
| 2.2.x   | ❌ No              |
| < 2.2   | ❌ No              |

## Reporting a Vulnerability

If you discover a security vulnerability in Alphabet Language, please report it responsibly.

### How to Report

1. **Email:** fraolteshome444@gmail.com
2. **Subject:** `[SECURITY] Alphabet Language Vulnerability`
3. **Include:**
   - Description of the vulnerability
   - Steps to reproduce
   - Potential impact
   - Suggested fix (if any)

### What to Expect

- **Acknowledgment:** Within 48 hours
- **Assessment:** Within 1 week
- **Fix:** Within 2 weeks for critical issues
- **Disclosure:** After fix is released

### Scope

**In Scope:**
- VM memory safety issues
- Sandbox escape vulnerabilities
- Code injection via crafted .abc files
- FFI security issues
- LSP server vulnerabilities

**Out of Scope:**
- Denial of service via infinite loops (use `--sandbox` mode)
- Issues in third-party dependencies (report upstream)
- Social engineering attacks

### Security Features

Alphabet includes several security features:

- **Sandbox mode:** `--sandbox` flag blocks FFI and file access
- **Memory limits:** Configurable memory limits for VM
- **Instruction limits:** Configurable instruction count limits
- **Input validation:** Type checking and bounds checking

### Best Practices

When running untrusted Alphabet code:

```bash
# Use sandbox mode
alphabet --sandbox untrusted.abc

# Set memory limit
alphabet --memory-limit=100M untrusted.abc

# Use instruction limit
alphabet --instruction-limit=1000000 untrusted.abc
```
