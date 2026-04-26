# Getting Started with Alphabet

**Welcome to Alphabet!** This guide will help you write your first programs in 10 minutes.

---

## Step 1: Install Alphabet

### One Command (Linux/macOS)

```bash
curl -fsSL https://raw.githubusercontent.com/fraol163/alphabet/main/install.sh | sh
```

This downloads, installs, and starts the REPL automatically.

### Windows

Download from [GitHub Releases](https://github.com/fraol163/alphabet/releases/latest).

---

## Step 2: Verify Installation

```bash
alphabet --version
```

**Expected output:**
```
Alphabet 2.3.3 (Native C++)
Developer: Fraol Teshome (fraolteshome444@gmail.com)
Compiled with C++17
```

---

## Step 3: Your First Program

Create a file named `hello.abc`:

```alphabet
#alphabet<en>
12 greeting = "Hello, World!"
z.o(greeting)
```

Run it:

```bash
alphabet hello.abc
```

**Output:**
```
Hello, World!
```

---

## Step 4: Understanding the Syntax

### Magic Header

Every `.abc` file must start with:

```alphabet
#alphabet<en>
```

This identifies the file as Alphabet code.

### Variables and Types

Alphabet uses **numeric type IDs**:

```alphabet
#alphabet<en>
5 x = 10              # int (type 5)
6 pi = 3.14           # float (type 6)
12 name = "Alphabet"  # string (type 12)
11 ok = (1 == 1)      # bool (type 11)
13 nums = [1, 2, 3]   # list (type 13)
14 cfg = {"k": 100}   # map (type 14)
```

### Output

Use `z.o()` to print:

```alphabet
z.o(x)        # Print variable
z.o("Hello")  # Print string
z.o(10 + 5)   # Print expression
```

### String Concatenation

```alphabet
z.o("x=" + 5)      # Output: x=5
z.o(5 + " items")  # Output: 5 items
```

### String Interpolation (f-strings)

```alphabet
5 name = "Alphabet"
5 age = 25
z.o(f"Hello, {name}!")          # Output: Hello, Alphabet!
z.o(f"You are {age} years old") # Output: You are 25 years old
```

---

## Step 5: Control Flow

### If-Else

```alphabet
5 x = 10
i (x > 5) {
  z.o("big")
} e {
  z.o("small")
}
```

### While Loop

```alphabet
5 i = 0
l (i < 5) {
  z.o(i)
  5 i = i + 1
}
```

### For Loop

```alphabet
l (5 j = 0 : j < 5 : j = j + 1) {
  z.o(j)
}
```

### Break and Continue

```alphabet
5 k = 0
l (k < 10) {
  5 k = k + 1
  i (k == 3) { b }      # break out of loop
  i (k % 2 == 0) { k }  # skip to next iteration
  z.o(k)
}
```

---

## Step 6: Functions

```alphabet
#alphabet<en>
m 5 factorial(5 num) {
  i (num <= 1) { r 1 }
  r num * factorial(num - 1)
}

z.o(factorial(5))  # Output: 120
```

---

## Step 7: Classes

```alphabet
#alphabet<en>
c Calculator {
  5 value = 0

  v m 5 add(5 x) {
    value = value + x
    r value
  }
}

15 calc = n Calculator()
z.o(calc.add(10))  # Output: 10
z.o(calc.add(5))   # Output: 15
```

---

## Step 8: Built-In Functions

```alphabet
#alphabet<en>
z.o(z.sqrt(144))    # 12
z.o(z.abs(-42))     # 42
z.o(z.pow(2, 10))   # 1024
z.o(z.len("hello")) # 5
```

---

## Next Steps

- [Complete Guide](COMPLETE_GUIDE.md) - Full language tutorial
- [Quick Reference](../QUICK_REFERENCE.md) - Cheat sheet
- [Standard Library](../stdlib/) - math.abc, io.abc
- [Examples](../examples/) - Example programs

---

**Happy Coding!**
