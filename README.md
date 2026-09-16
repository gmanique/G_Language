# ⚠️ Disclaimer: This is an experimental language, code might be messy

# G_Language Compiler 🛠️

[![Version](https://img.shields.io/badge/version-0.1.0-blue)](https://github.com/gmanique/G_Language)
[![License](https://img.shields.io/badge/license-MIT-green)](LICENSE)

Welcome to the official repository for **G_Language**, my personal programming language and its **compiler**.  
This project is under active development and aims to create a simple yet powerful language, with a compiler built completely from scratch in modern C++23.

---

## 📌 Current Version

- **0.1.0**

---

## 🕰️ Version History & Roadmap

- **`0.0.1`** – File reading & basic CLI setup
- **`0.1.0`** – Lexer implementation
- **`0.2.0`** – Parser & AST construction
- **`0.3.0`** – Semantic analysis & type checking
- **`0.4.0`** – Assembly code generation
- **`1.0.0`** – First stable release (direct executable output)

---

## 🚀 Project Goal

Develop a **fully functional compiler** capable of translating G_Language code into executable code.  
Completely written in C++23.

Key features:

- ✅ Robust lexer and parser
- 🔧 Flexible AST for expressions and instructions
- ⏳ Optimized code generation
- 🧩 Support for control structures (`if`, `for`, `while`, …)
- ⚡ Easily extensible for new features

---

## ⚙️ Current Status

| Component         | Status       |
| ----------------- | ------------ |
| Lexer             | ✅ completed |
| Parser / AST      | ⏳ upcoming  |
| Basic expressions | ⏳ upcoming  |
| Control flow      | ⏳ upcoming  |
| Code generation   | ⏳ upcoming  |

---

## 📦 Installation & Build

Clone the repository:

```bash
git clone [https://github.com/gmanique/G_Language.git](https://github.com/gmanique/G_Language.git)
cd G_Language
```

---

## Build the compiler

```bash
cmake -B build
cmake --build build
```

---

## 💻 Usage

To compile a G_Language program:

```bash
./build/glang <file_to_compile>
```

Then run the output:

```bash
./exec.out
```

---

## 📝 Versioning

I use **Semantic Versioning (SemVer)**:

- `0.x.y` → experimental / development

- `1.0.0` → first stable release

---

## 🤝 Contributing

This project is open-source under the MIT License and currently under active development. I’m working on it solo, but I’m happy to hear any ideas or suggestions!

---

✨ Thank you for your interest in G_Language!
