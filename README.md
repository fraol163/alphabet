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
**Developed by Fraol Teshome** (`fraolteshome444@gmail.com`)

[![License: MIT](https://img.shields.io/badge/License-MIT-yellow.svg)](https://opensource.org/licenses/MIT)
[![Build Status](https://github.com/fraol163/alphabet/actions/workflows/build.yml/badge.svg)](https://github.com/fraol163/alphabet/actions)
[![Platform](https://img.shields.io/badge/platform-Windows%20%7C%20Linux-lightgrey.svg)](#-global-installation)

**Alphabet** is a minimalist, high-density programming language where every keyword is a single character and every type is a numeric ID. Designed for developers who value logical elegance and maximum code density.

---

## 🚀 Why Alphabet?
In modern software development, verbosity often hides logic. Alphabet removes the "noise" by reducing the language to its core mathematical and logical symbols. 
- **Zero Boilerplate:** No long words like `public`, `static`, or `function`.
- **Pure Logic:** Every keystroke contributes directly to the execution of your program.
- **Source Protection:** The internal engine is distributed as machine-code to protect intellectual property.

---

## 📖 The Alphabet Dictionary (Keywords)
Every control structure in Alphabet is represented by a single character.

| Letter | Name | Description |
| :--- | :--- | :--- |
| **`a`** | **Abstract** | Defines a base class that cannot be instantiated. |
| **`b`** | **Break** | Exits the current loop immediately. |
| **`c`** | **Class** | Defines a new object blueprint. |
| **`e`** | **Else** | Alternative path for an `i` (If) statement. |
| **`h`** | **Handle** | Catches and manages errors from a `t` (Try) block. |
| **`i`** | **If** | Starts a conditional logic check. |
| **`j`** | **Interface** | Defines a contract for other classes to follow. |
| **`k`** | **Continue** | Skips to the next cycle of a loop. |
| **`l`** | **Loop** | Repeats a block of code while a condition is true. |
| **`m`** | **Method** | Defines a function or action inside a class. |
| **`n`** | **New** | Instantiates a live object from a class. |
| **`p`** | **Private** | Restricts a member's visibility to its own class. |
| **`r`** | **Return** | Exits a method and sends back a value. |
| **`s`** | **Static** | Defines a member belonging to the class itself. |
| **`t`** | **Try** | Wraps code to monitor for potential errors. |
| **`v`** | **Public** | Makes a member accessible from anywhere. |
| **`z`** | **System** | Gateway to the standard library (Output, Files, etc.). |

---

## 🔢 The Numeric Type Map (1-50)
Types are identified by numbers to maintain consistent density and speed.

| ID | Data Type | ID | Data Type |
| :--- | :--- | :--- | :--- |
| **1** | 8-bit Integer (`i8`) | **8** | Default Float (`float`) |
| **2** | 16-bit Integer (`i16`) | **9** | High-precision Decimal |
| **3** | 32-bit Integer (`i32`) | **10**| Complex Number |
| **4** | 64-bit Integer (`i64`) | **11**| Boolean (`True`/`False`) |
| **5** | Default Integer (`int`) | **12**| String (Text) |
| **6** | 32-bit Float (`f32`) | **13**| List (Dynamic Array) |
| **7** | 64-bit Float (`f64`) | **14**| Map (Key-Value Store) |

### 🛠 User-Defined Types (15 - 50)
Numbers **15 through 50** are reserved for your own classes. They are assigned automatically by the compiler in the order they are defined.
- **ID 15:** The first class defined in your code.
- **ID 16:** The second class defined in your code.
- ...and so on.

---

## 📦 Acquiring Alphabet (Automated CI/CD)
Alphabet uses an **Enterprise-Grade Build Pipeline**. Native binaries for every operating system are generated automatically via GitHub Actions Cloud Servers.

### How to Download:
1.  Navigate to the **[Actions](https://github.com/fraol163/alphabet/actions)** tab of this repository.
2.  Click on the latest successful run of the **"Build Alphabet Language"** workflow.
3.  Scroll to the **Artifacts** section at the bottom.
4.  Download the package matching your OS:
    -   `alphabet-ubuntu-latest` (For Linux)
    -   `alphabet-windows-latest` (For Windows `.exe`)

---

## 🛠 Global Installation

### 🐧 For Linux / WSL Users
1.  Unzip the downloaded `alphabet-ubuntu-latest.zip`.
2.  Open your terminal in that folder.
3.  Grant execution rights:
    ```bash
    chmod +x alphabet
    ```
4.  Move it to your system path to use it globally:
    ```bash
    sudo mv alphabet /usr/local/bin/alphabet
    ```
5.  **Verify:** Type `alphabet -v` from any directory.

### 🪟 For Windows Users
1.  Unzip `alphabet-windows-latest.zip` to find `alphabet.exe`.
2.  Create a dedicated folder (e.g., `C:\Alphabet`) and move `alphabet.exe` there.
3.  **Add to System PATH:**
    -   Press the **Start** key and type "Environment Variables".
    -   Click **"Edit the system environment variables"**.
    -   Click **"Environment Variables"** at the bottom.
    -   Under **"System Variables"**, find and select **"Path"**, then click **"Edit"**.
    -   Click **"New"** and type `C:\Alphabet`.
    -   Click **OK** on all windows.
4.  **Verify:** Open a new PowerShell and type `alphabet -v`.

---

## 🚀 Quick Start Example
Create a file named `demo.abc`:
```alphabet
// Object-Oriented Logic
c A {
  v m 1 g() { r 10 }
}

c B ^ A {
  v m 1 g() { r 20 }
}

15 o = n B()
z.o(o.g()) // Outputs 20
```
Run it:
```bash
alphabet demo.abc
```

---

## 🤝 Support
Developed with ❤️ by **Fraol Teshome**.
For technical inquiries or partnerships, contact: **fraolteshome444@gmail.com**
