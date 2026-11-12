# The Myst Programming Language: Official Guide

Welcome to the **Myst** language reference. Myst is a native, systems-level programming language designed to compile directly to LLVM IR without a garbage collector. It gives you precise control over memory layout, data types, and performance while keeping syntax clean, minimal, and indentation-based.

---

## 1. Basic Types and Variables

Myst supports standard primitive types aligned directly with LLVM's underlying primitives:

- **`int`**: Signed 32-bit integer (`i32`).
- **`decimal`**: Double-precision 64-bit floating-point number (`double`).
- **`char`**: 8-bit character / byte (`i8`).

### Variable Declarations

- Use **`var`** only when you want the compiler to infer the type from the assigned value.
- If you explicitly state the type, **`var` is omitted entirely**.
- No semicolons are required at the end of lines.

```myst
// Compiler infers the type
var inferredInt = 45

// Explicit type declaration (no 'var' needed)
int explicitInt = 45
decimal y = 342.3
char c = 'c'
```

---

## 2. Functions and Control Flow

### Functions

Functions are defined using the `func` keyword, ending with a **colon (`:`)** before entering the indented block.

```myst
func add(int a, int b) -> int:
    return a + b

func printHello():
    // Void-returning functions omit the return type arrow
    return
```

### Control Flow

Control statements like `if`, `else`, and `while` **do not require parentheses**, use significant indentation, and **must end with a colon (`:`)**.

```myst
func checkNumber(int x):
    while x > 0:
        if x == 55:
            // Do something special
            x = x * 5
        else:
            x = x - 1
```

---

## 3. Top-Level Execution (No `main` Required)

Unlike traditional systems languages like C or C++, **Myst does not require a `main()` function**. 

Myst supports **top-level execution** out of the box. Code written at the root level of a file executes sequentially from top to bottom on startup. This gives Myst a clean, modern feel suitable for both scripting and building large systems modules without boilerplate entry-point wrappers.

```myst
// Top-level statements run immediately
int x = 10
int y = 20
print(x + y)
```

---

## 4. References and Memory Management

Because Myst is a systems language without a garbage collector, managing how data is passed into functions is critical for performance:

- **Pass by Value:** By default, primitive types and small data structures are copied when passed into functions.
- **Pass by Reference (`ref`):** Placed **before the data type** in parameter lists to allow functions to mutate the caller's data directly without copying.

```myst
func increment(ref int val):
    val = val + 1
```

---

## 5. Custom Types (Containers)

Custom data aggregates in Myst are called **containers** and are defined by ending the declaration header with a **colon (`:`)**. Under the hood, these map directly to LLVM identified structs with strict hardware alignment and padding rules.

```myst
container Person:
    int id
    int age
    int tier
    char grade
    char status
```

### Creating and Using Instances

You can instantiate containers using their constructor and access or mutate their fields directly.

```myst
Person p = Person(1, 30, 3, 'A', '1')
p.age = 31
print(p.age)
```

---

## 6. Modules, Imports, and Exports

Myst uses a clean, file-based module system using standard **dot notation** for paths without file extension strings.

### Exporting Symbols

Exports happen strictly **at the very beginning of the file**, explicitly exposing its public API.

```myst
// math.myst
export { Vector, scale }

container Vector:
    int x
    int y

func scale(Vector v, int factor) -> Vector:
    return Vector(v.x * factor, v.y * factor)
```

### Importing Modules

Other files import functionality using dot notation at the top of the file.

```myst
// main.myst
import math as M

M::Vector v = M::Vector(10, 20)
M::Vector scaled = M::scale(v, 2)
```

---

## 7. A Complete Example Program

Here is what a complete, working Myst script looks like, leveraging **top-level execution** directly without needing an explicit `main()` entry point:

```myst
// factorial.ms
func computeFactorial(int n) -> int:
    int result = 1
    int i = 1

    while i <= n:
        result = result * i
        i = i + 1

    return result

// Top-level execution runs immediately
int val = 5
int fact = computeFactorial(val)
print(fact)
```