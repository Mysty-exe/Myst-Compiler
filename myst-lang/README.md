# Myst Language Support

`myst-lang` is the official Visual Studio Code extension for the **Myst Programming Language**. It provides rich syntax highlighting and language configuration support for `.ms` source files.

## Features

### Syntax Highlighting

The extension provides clean, theme-aware syntax highlighting for all core constructs of the Myst language:

- **Keywords & Control Flow:** `func`, `var`, `if`, `elseif`, `else`, `while`, `return`, `and`, `or`, `not`
- **Primitive Types:** `char`, `int`, `decimal`, `bool`
- **Literals & Constants:**
  - Booleans (`true`, `false`)
  - Integers and decimal numbers
  - Single-quoted character literals (e.g., `'a'`, `'\n'`)
- **Comments:** Single-line comments starting with `//`
- **Operators:** Arithmetic (`+`, `-`, `*`, `/`, `%`, `^`), comparison (`==`, `!=`, `<`, `>`, `<=`, `>=`), logical (`&&`, `||`), and assignment (`=`).

---

## Example Code

```ms
func add(int x) -> int:
    // Initialize a local variable
    var count = 0

    while count < x:
        if count == 5:
            return 32
        else:
            count = count + 1

    return x
```
