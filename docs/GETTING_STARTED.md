# Getting Started with Alphabet

**Welcome to Alphabet!** This guide will help you write your first programs in 10 minutes.

---

## Prerequisites

- A computer (Windows, macOS, or Linux)
- No programming experience required!

---

## Step 1: Install Alphabet

### Option A: Download Pre-built Binary (Recommended)

1. Visit [GitHub Actions](https://github.com/fraol163/alphabet/actions)
2. Click the **latest build** at the top
3. Scroll to **Artifacts** section
4. Download your OS:
   - **Linux:** `alphabet-linux`
   - **macOS:** `alphabet-macos`
   - **Windows:** `alphabet-windows.exe`

### Option B: Build from Source

See [INSTALLATION.md](INSTALLATION.md) for detailed build instructions.

---

## Step 2: Verify Installation

Open a terminal/command prompt and run:

```bash
alphabet --version
```

**Expected output:**
```
Alphabet 2.0.0 (Native C++)
Developer: Fraol Teshome (fraolteshome444@gmail.com)
Compiled with C++17
```

---

## Step 3: Your First Program

### Hello World

1. Create a file named `hello.abc`:

```alphabet
#alphabet<en>
12 greeting = "Hello, World!"
z.o(greeting)
```

2. Run it:

```bash
alphabet hello.abc
```

**Output:**
```
Hello, World!
```

**Congratulations! You just wrote your first Alphabet program! 🎉**

---

## Step 4: Understanding the Syntax

### Magic Header

Every `.abc` file must start with:

```alphabet
#alphabet<en>
```

This identifies the file as Alphabet code.

### Variables

Alphabet uses **numeric type IDs**:

```alphabet
#alphabet<en>
5 x = 10          # int
6 pi = 3.14       # float
12 name = "Alphabet"  # string
11 ok = (1 == 1)  # bool
```

### Output

Use `z.o()` to print:

```alphabet
z.o(msg)      # Print variable
z.o("Hello")  # Print string
```

---

## Step 5: Control Flow

### If Statements

```alphabet
i (x > 5) {
  z.o("x is greater")
} e {
  z.o("x is 5 or less")
}
```

### Loops

```alphabet
5 i = 0
l (i < 5) {
  z.o(i)
  5 i = i + 1
}
```

---

## Step 6: Next Steps

- [Tutorial](TUTORIAL.md) - Step-by-step examples
- [Reference](REFERENCE.md) - Complete syntax
- [Installation](INSTALLATION.md) - Detailed setup

---

**Happy Coding! 🚀**
