# Myst Compiler

A custom, cross-platform programming language compiler written from scratch in C++ that targets **LLVM IR**. Myst combines clean, expressive, indentation-sensitive syntax with high-performance native code generation through LLVM and Clang.

## Features

- **LLVM IR Backend:** Generates LLVM Intermediate Representation (IR), enabling optimization and native machine code generation.
- **Cross-Platform Support:** Supports native compilation targets for Windows (`x86_64-pc-windows-msvc`) and Linux (`x86_64-pc-linux-gnu`).
- **Primitive Type System:** Supports integers (`int`), doubles (`decimal`), booleans (`bool`), single-byte characters (`char`).
- **Structured Control Flow:** Provides indentation-based blocks, conditional statements (`if`, `elseif`, `else`), and loops (`while`).
- **Functions and Recursion:** Supports function declarations, return types, and recursive function calls.
- **Runtime Error Handling:** Includes runtime safety checks for illegal operations, such as division by zero, to prevent undefined behavior and handle errors gracefully.

## Syntax Overview

Myst source files use the `.ms` (or `.myst`) extension. Here is an example demonstrating function declarations, conditional statements, recursion, variable declarations, and output.

```ms
func factorial(int n) -> int:
    if (n <= 1):
        return 1

    return n * factorial(n - 1)


var ans = factorial(5)
print(ans)

// Expected output: 120
```

## Architecture

Myst follows a multi-stage compilation pipeline.

1. **Lexing and Parsing:** Tokenizes source files, parses indentation-based blocks, constructs an Abstract Syntax Tree (AST), and validates syntax.
2. **Semantic Analysis:** Performs type checking, scope resolution, and control-flow verification.
3. **LLVM IR Generation:** Translates the validated AST into LLVM Intermediate Representation, producing `.ll` files in Static Single Assignment (SSA) form.
4. **Native Code Generation:** Uses Clang to compile the generated LLVM IR into a native executable for the target platform.

## Building and Usage

### Prerequisites

- A C++ compiler supporting C++17 or later.
- LLVM and Clang installed and available through your system's `PATH`.
- CMake and any additional build dependencies required by the project.

### 1. Build the Compiler

Configure and build the project using its CMake configuration.

```bash
cmake -S . -B build
cmake --build build --config Release
```

The exact compiler executable path depends on your build configuration.

### 2. Generate LLVM IR

Compile a Myst source file into LLVM IR.

```bash
myst source.ms -o main
```

This generates `main.exe` from `source.ms`.

### 3. Run the Program

On Linux:

```bash
./main
```

On Windows:

```powershell
.\main.exe
```

## Companion Tools

- [**Myst Language Extension**](myst-lang): A Visual Studio Code extension for Myst, providing syntax highlighting, comment handling, and configuration for `.ms` source files.

## Future Work

- Implicit and Explicit Typecasting
- Importing other `.ms` files
- Pass by value and reference
- Strings and Lists/Arrays
- For loops
- Custom Types
