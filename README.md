```text
            d8b            d8b                 d8b                     
           88P            ?88                 ?88                d8P  
          d88              88b                 88b            d888888P
 d888b8b  888  ?88,.d88b,  888888b  d888b8b    888888b  d8888b  ?88'  
d8P' ?88  ?88  `?88'  ?88  88P `?8bd8P' ?88    88P `?8bd8b_,dP  88P   
88b  ,88b  88b   88b  d8P d88   88P88b  ,88b  d88,  d8888b      88b   
`?88P'`88b  88b  888888P'd88'   88b`?88P'`88bd88'`?88P'`?888P'  `?8b  
                 88P'                                                 
                d88                                                   
                ?8P
```

# Alphabet Programming Language 🔤
**The High-Density Turing-Complete Syntax**  
**Developed by Fraol Teshome** (`fraolteshome444@gmail.com`)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/fraol163/alphabet/actions/workflows/build.yml/badge.svg)](https://github.com/fraol163/alphabet/actions)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)](#-global-installation)

**Alphabet** is a minimalist, high-density programming language where every keyword is a single character and every type is a numeric ID. Designed for developers who value logical elegance, speed, and maximum code density.

---

## 📖 1. The Alphabet Dictionary (Keywords)
Alphabet uses a strict one-character keyword system. Every letter has a specific structural purpose.

| Letter | Name | Usage | Description |
| :--- | :--- | :--- | :--- |
| **`a`** | **Abstract** | `a c A { ... }` | Defines a class that cannot be created directly. |
| **`b`** | **Break** | `b` | Exits the current loop immediately. |
| **`c`** | **Class** | `c A { ... }` | Defines a new object blueprint. |
| **`e`** | **Else** | `i (...) { } e { }` | The alternative path when an `i` check is false. |
| **`h`** | **Handle** | `h (15 e) { }` | Catches and manages errors from a `t` block. |
| **`i`** | **If** | `i (x > 0) { }` | Starts a conditional logic check. |
| **`j`** | **Interface**| `j J { m 1 f() }` | Defines a method contract for classes to join. |
| **`k`** | **Continue**| `k` | Skips the rest of the loop and starts the next cycle. |
| **`l`** | **Loop** | `l (x > 0) { }` | Repeats code as long as the condition is true. |
| **`m`** | **Method** | `v m 1 f() { }` | Defines a function or action inside a class. |
| **`n`** | **New** | `15 o = n A()` | Instantiates a live object from a class. |
| **`p`** | **Private** | `p 1 x = 10` | Restricts access to within the same class. |
| **`r`** | **Return** | `r 10` | Exits a method and sends back a value. |
| **`s`** | **Static** | `s 1 x = 5` | Defines a member shared by the entire class. |
| **`t`** | **Try** | `t { ... }` | Monitors a block of code for potential errors. |
| **`v`** | **Public** | `v m 1 f() { }` | Makes a member accessible from anywhere. |
| **`z`** | **System** | `z.o("Hi")` | Gateway to built-in system powers (I/O, Files). |

---

## 🔢 2. The Numeric Type Map (1-50)
Alphabet identifies data types by number to maximize density.

| ID | Data Type | Usage Example |
| :--- | :--- | :--- |
| **1** | **i8** (8-bit Int) | `1 small = 10` |
| **2** | **i16** (16-bit Int)| `2 medium = 1000` |
| **3** | **i32** (32-bit Int)| `3 standard = 50000` |
| **4** | **i64** (64-bit Int)| `4 large = 90000000` |
| **5** | **int** (Generic)   | `5 x = 10` |
| **6** | **f32** (Float)     | `6 p = 3.14` |
| **7** | **f64** (Double)    | `7 d = 3.14159` |
| **8** | **float** (Generic) | `8 f = 0.5` |
| **9** | **dec** (Decimal)   | `9 money = 99.99` |
| **10**| **cpx** (Complex)   | `10 num = 1+2j` |
| **11**| **bool** (Boolean)  | `11 ok = (1 == 1)` |
| **12**| **str** (String)    | `12 s = "Hello"` |
| **13**| **list** (Array)    | `13 a = [1, 2, 3]` |
| **14**| **map** (Hash)      | `14 m = {"id": 1}` |
| **15-50**| **obj** (Custom) | `15 user = n Person()` |

*Note: IDs 15-50 are assigned automatically to your custom classes in the order they are defined.*

---

## 📦 3. Acquiring Alphabet (The Cloud Pipeline)
Alphabet is distributed as a high-performance native binary. You do not need to install source code or Python.

### How to Download:
1.  Go to the **[Actions](https://github.com/fraol163/alphabet/actions)** tab of this repository.
2.  Select the **latest successful build** at the top of the list.
3.  Scroll to the **"Artifacts"** section at the bottom.
4.  Download the file for your system:
    -   `alphabet-ubuntu-latest` (For Linux)
    -   `alphabet-windows-latest` (For Windows `.exe`)

---

## 🛠 4. Global Installation

### 🐧 For Linux / macOS
1.  Unzip the artifact to find the `alphabet` binary.
2.  Grant execution rights: `chmod +x alphabet`
3.  Move to global bin: `sudo mv alphabet /usr/local/bin/alphabet`
4.  **Test:** Type `alphabet -v` in your terminal.

### 🪟 For Windows
1.  Unzip the artifact to find `alphabet.exe`.
2.  Create folder `C:\Alphabet` and move the `.exe` inside.
3.  **Setup Path:**
    -   Search Start for **"Environment Variables"** -> Click **Edit**.
    -   Select **Path** -> Click **Edit** -> Click **New**.
    -   Type `C:\Alphabet` and click **OK**.
4.  **Test:** Open PowerShell and type `alphabet -v`.

---

## 🚀 5. Getting Started Examples

### The Hello World
```alphabet
12 h = "Hello Alphabet!"
z.o(h)
```

### Advanced OOP Logic
```alphabet
c A {
  v m 1 g() { r 10 }
}

c B ^ A {
  v m 1 g() { r 20 }
}

15 o = n B()
z.o(o.g()) // Returns 20 (Polymorphism)
```

---

## 🤝 Contact & Presentation
Developed with ❤️ by **Fraol Teshome**.  
For tech presentation inquiries or partnerships:  
📧 **fraolteshome444@gmail.com**
