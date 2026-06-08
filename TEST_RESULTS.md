# Alphabet Language - Test Results

**Version:** 2.3.5
**Status:** All tests passing

---

## Test Summary

**34/34 ctest + 257 test files**

| Test Suite | Tests | Status |
|------------|-------|--------|
| LexerTests | Unit tests | ✅ Pass |
| ParserTests | Unit tests | ✅ Pass |
| VMTests | Unit tests | ✅ Pass |
| LanguageTests | Language features | ✅ Pass |
| Golden: hello | Output match (en, am, es, fr, de) | ✅ Pass |
| Golden: arithmetic | Output match (en, am, es, fr, de) | ✅ Pass |
| Golden: class_method | Output match (en, am, es, fr, de) | ✅ Pass |
| Golden: if_else | Output match (en, am, es, fr, de) | ✅ Pass |
| Golden: loop | Output match (en, am, es, fr, de) | ✅ Pass |
| Golden: dump_bytecode | Output match | ✅ Pass |
| Golden: stdlib_io | Output match | ✅ Pass |
| Golden: stdlib_list | Output match | ✅ Pass |
| Golden: stdlib_math | Output match | ✅ Pass |
| Golden: stdlib_string | Output match | ✅ Pass |

---

## Running Tests

```bash
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build
cmake --build build -j$(nproc)
cd build && ctest --output-on-failure
```

---

## CI/CD

- GitHub Actions: multi-platform (Ubuntu, Windows, macOS)
- AddressSanitizer (ASan) builds enabled for memory safety
- clang-format enforcement

---

## Project Stats

- **~11K source lines** total (9 .cpp, 10 .h in src/, plus 3 test .cpp files)
- **19 single-letter keywords** (a-s)
- **4 standard library modules** (math, io, string, list)
- **30 golden test files** (6 base scenarios × 5 languages + 5 stdlib + 1 dump_bytecode)

---

## Golden Test Files

```
tests/golden_files/
├── hello.abc / hello.expected
├── hello_{am,es,fr,de}.abc / hello_{am,es,fr,de}.expected
├── arithmetic.abc / arithmetic.expected
├── arithmetic_{am,es,fr,de}.abc / arithmetic_{am,es,fr,de}.expected
├── class_method.abc / class_method.expected
├── class_method_{am,es,fr,de}.abc / class_method_{am,es,fr,de}.expected
├── if_else.abc / if_else.expected
├── if_else_{am,es,fr,de}.abc / if_else_{am,es,fr,de}.expected
├── loop.abc / loop.expected
├── loop_{am,es,fr,de}.abc / loop_{am,es,fr,de}.expected
├── dump_bytecode.abc / dump_bytecode.expected
├── stdlib_io.abc / stdlib_io.expected
├── stdlib_list.abc / stdlib_list.expected
├── stdlib_math.abc / stdlib_math.expected
└── stdlib_string.abc / stdlib_string.expected
```
