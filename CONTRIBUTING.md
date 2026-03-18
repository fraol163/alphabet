# Contributing to Alphabet

**Thank you for your interest in contributing!** This guide will help you get started.

---

## Quick Links

- [Code of Conduct](#code-of-conduct)
- [Getting Started](#getting-started)
- [Development Setup](#development-setup)
- [Making Changes](#making-changes)
- [Pull Request Guidelines](#pull-request-guidelines)

---

## Code of Conduct

### Our Pledge

We pledge to make participation in our community a harassment-free experience for everyone.

### Expected Behavior

- Be respectful and inclusive
- Accept constructive criticism
- Focus on what's best for the community

### Unacceptable Behavior

- Harassment or discrimination
- Trolling or insulting comments
- Publishing others' private information

### Enforcement

Report abusive behavior to fraolteshome444@gmail.com

---

## Getting Started

### 1. Fork the Repository

Visit https://github.com/fraol163/alphabet and click "Fork"

### 2. Clone Your Fork

```bash
git clone https://github.com/YOUR_USERNAME/alphabet.git
cd alphabet
```

### 3. Set Upstream Remote

```bash
git remote add upstream https://github.com/fraol163/alphabet.git
git fetch upstream
```

---

## Development Setup

### Prerequisites

- CMake 3.16+
- C++17 compiler (GCC 9+, Clang 10+, or MSVC 2019+)
- Git

### Build from Source

```bash
# Clone repository
git clone https://github.com/fraol163/alphabet.git
cd alphabet

# Create build directory
mkdir build && cd build

# Configure
cmake -DCMAKE_BUILD_TYPE=Debug ..

# Build
make -j$(nproc)

# Run tests
ctest --output-on-failure
```

---

## Making Changes

### 1. Find an Issue

- Browse [existing issues](https://github.com/fraol163/alphabet/issues)
- Look for labels: `good first issue`, `help wanted`, `bug`

### 2. Create Branch

```bash
git checkout -b feature/your-feature-name
```

### 3. Make Your Changes

```bash
# Edit files
# ...

# Stage changes
git add src/include/lexer.cpp

# Commit
git commit -m "fix: Resolve memory leak in lexer"
```

### 4. Test Your Changes

```bash
# Run all tests
cd build
ctest --output-on-failure

# Run specific test
./test_lexer
```

---

## Pull Request Guidelines

### Before Submitting

- [ ] Tests pass (`ctest --output-on-failure`)
- [ ] Code is formatted
- [ ] Documentation updated
- [ ] Commit message follows conventions

### PR Title Format

```
type: Brief description
```

**Types:**
- `feat:` New feature
- `fix:` Bug fix
- `docs:` Documentation changes
- `style:` Code style changes
- `refactor:` Code refactoring
- `test:` Test additions
- `chore:` Build/config changes

### PR Description Template

```markdown
## Description
Brief description of changes

## Related Issue
Fixes #123

## Testing
- [ ] Tests pass
- [ ] Added new tests if needed

## Checklist
- [ ] Code follows style guidelines
- [ ] Documentation updated
```

---

## Coding Standards

### C++ Style

**Naming Conventions:**
- Classes: `PascalCase` (e.g., `TokenType`)
- Functions: `snake_case` (e.g., `scan_tokens`)
- Variables: `snake_case` with `_` for members (e.g., `source_`)

**Code Style:**
```cpp
// Use auto for clarity
auto token = tokens_.back();

// Use std::string_view for read-only strings
void process(std::string_view input);

// Use const correctness
const std::string& get_name() const;
```

---

## Testing Guidelines

### Running Tests

```bash
# All tests
ctest --output-on-failure

# Specific test
./test_lexer

# With verbose output
ctest -V
```

---

## Areas Needing Contribution

### High Priority

- [ ] VS Code extension improvements
- [ ] More real-world examples
- [ ] Package manager support (Homebrew, apt)
- [ ] Debugger implementation

### Medium Priority

- [ ] Standard library expansion
- [ ] FFI documentation
- [ ] Translations

---

## Questions?

### Getting Help

- **General questions:** [GitHub Discussions](https://github.com/fraol163/alphabet/discussions)
- **Bug reports:** [GitHub Issues](https://github.com/fraol163/alphabet/issues)
- **Email:** fraolteshome444@gmail.com

---

## License

By contributing, you agree that your contributions will be licensed under the MIT License.

---

**Thank you for contributing to Alphabet! 🚀**
