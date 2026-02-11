# Alphabet: The Complete Master Guide
**By Fraol Teshome** (`fraolteshome444@gmail.com`)

Welcome to the definitive guide for the Alphabet programming language. This document provides a step-by-step breakdown of how to use, distribute, and manage the Alphabet ecosystem.

---

## 1. Installation & Global Setup
Alphabet is distributed as a high-performance native binary.

### A. Linux & macOS
1. **Download:** Get the `alphabet` binary from the repository's **Actions** tab (Artifacts).
2. **Permissions:** Open your terminal and run `chmod +x alphabet`.
3. **Install:** Move it to your local bin: `sudo mv alphabet /usr/local/bin/alphabet`.
4. **Test:** Run `alphabet -v` to see your version and developer info.

### B. Windows
1. **Download:** Get `alphabet.exe` from the **Actions** tab.
2. **Move:** Create a folder `C:\Alphabet` and place the `.exe` inside.
3. **Environment:** Add `C:\Alphabet` to your system's **Path** variable in the Environment Variables settings.
4. **Test:** Open PowerShell and run `alphabet -v`.

---

## 2. The Language "Dictionary" (One-by-One)
Every keyword in Alphabet is a single character.

- **`a` (Abstract):** Marks a class that can only be inherited from, not created directly.
- **`b` (Break):** Immediately stops a loop and moves to the next part of the program.
- **`c` (Class):** The blueprint for objects. Used to create your own types (15-50).
- **`e` (Else):** The alternative code path when an `i` (If) check fails.
- **`h` (Handle):** The error-catcher. It manages what happens when a `t` (Try) block fails.
- **`i` (If):** The logic checker. Evaluates a condition to decide if code should run.
- **`j` (Interface):** Defines a strict "Contract" of methods that a class must implement.
- **`k` (Continue):** Skips the rest of the current loop cycle and starts the next one.
- **`l` (Loop):** The universal repetition tool. Replaces 'while' and 'for'.
- **`m` (Method):** A function or behavior defined inside a class.
- **`n` (New):** The "Instantiator." It turns a class blueprint into a live object.
- **`p` (Private):** Security modifier. Members marked `p` are hidden from the outside world.
- **`r` (Return):** Sends a value back to whoever called the method.
- **`s` (Static):** Class-level data. Shared by all objects of that class.
- **`t` (Try):** The monitor. It watches code for potential runtime errors.
- **`v` (Public):** The open modifier. Members marked `v` are usable by anyone.
- **`z` (System):** The built-in Standard Library (Output, Input, Files).

---

## 3. The Numeric Type Map (One-by-One)
Instead of names like "string" or "int", Alphabet uses IDs 1-50.

- **1:** 8-bit Integer (Small whole numbers).
- **2:** 16-bit Integer.
- **3:** 32-bit Integer (The standard whole number).
- **4:** 64-bit Integer (Very large whole numbers).
- **5:** Generic Integer (Automatic sizing).
- **6:** 32-bit Float (Decimal numbers).
- **7:** 64-bit Float (High-precision decimals).
- **8:** Default Float (Standard decimal).
- **9:** High-precision Decimal (For financial or scientific math).
- **10:** Complex Number (Math involving imaginary units).
- **11:** Boolean (True/False logic).
- **12:** String (Text and characters).
- **13:** List (A dynamic array of multiple values).
- **14:** Map (A dictionary of Key-Value pairs).
- **15-50:** Custom Objects (Assigned automatically to your classes).

---

## 4. Advanced Logic Examples

### Inheritance & Polymorphism
```alphabet
c A { v m 1 g() { r 10 } }
c B ^ A { v m 1 g() { r 20 } }
```

### Encapsulation (Private Data)
```alphabet
c S { p 1 x = 100 } // x is invisible to the outside
```

### Exception Handling
```alphabet
t { z.f("none.txt") } h (15 e) { z.o(e) }
```

---

## 5. Security & Build Infrastructure
To keep your logic private, Alphabet uses a **Source-to-C++ Machine-Code** pipeline.

- **The Builder:** `setup_dist.sh` uses the Nuitka compiler to hide your code.
- **The Cloud:** GitHub Actions builds your binaries on remote servers so you don't need a Windows PC.
- **The Release:** Only binaries are shared with users; the Python logic is never exposed.

---
Developed by **Fraol Teshome**. All rights reserved.
