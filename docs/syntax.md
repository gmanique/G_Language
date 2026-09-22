# G_Language syntax

Welcome to the official syntax documentation for the .gg programming language. `.gg` is a statically typed, systems-level language featuring a C- and Go-like syntax designed for clarity and performance.

---

## 1. File structure and imports

A `.gg` source file consists of top-level declarations: **imports**, **structs**, **enums**, and **functions**.
To include types or functions declared in another file, use the `import` keyword:

```c
import "structures.gg";
import "utils.gg";
```

Note: Symbols from an imported file are available file-wide,
regardless of where the import statement is placed within the file.

---

## 2. Reserved keywords

| Keyword   | Description                                                        |
| --------- | ------------------------------------------------------------------ |
| import    | Includes an external source file.                                  |
| struct    | Declares a composite data structure.                               |
| enum      | Declares an enumeration of named constants. (Note implemented yet) |
| fun       | Declares a function.                                               |
| if / else | Conditional execution branch.                                      |
| while     | Conditional loop execution.                                        |
| return    | Terminate function execution and optionally return a value.        |
| syscall   | Performs a direct kernel system call.                              |

---

## 3. Type System

### Primitive Types

- Signed Integers: i8, i16, i32, i64
- Unsigned Integers: u8, u16, u32, u64
- String: string (pointer to a null-terminated sequence of characters)

### Pointers

Pointers are declared by appending one or more asterisks * to the type:

- `i8* ptr` : Pointer to an 8-bit integer.
- `AstNode** node_ptr` : Pointer to a pointer of a struct.

### Arrays

- Fixed-size Array: `i8[5] array`; _(Contiguous allocation of 5 bytes)_
- Unsized Array / Slice: `Token[] tokens`; _(Mainly used as function parameters)_

---

## 4. Custom Type Declarations

### Structs

Used to group related fields under a single named type.

```c
struct Pos {
    i16 col;
    i16 line;
}

struct Token {
    i8 type;
    string value;
    Pos pos;
}
```

### Enums (Not implemented yet)

Used to define a discrete set of named integral constants.

```c
enum TokenType {
    WORD,
    NUMBER,
    STRING,
    OPERATOR
}
```

---

## 5. Functions

Functions are introduced using the `fun` keyword (because life is fun, isn't it ?).
Parameters follow the `Type name` syntax.
The return type is specified between the closing parameter parenthesis `)` and the opening brace `{`.
If omitted, the function defaults to a void return type.

```c
// Function with no return value (void)
fun print_separator() {
    write(1, "---\n", 4);
}

// Function with parameters and an i8 return type
fun stringHasChar(string str, i8 character) i8 {
    i32 i = 0;
    while (str[i]) {
        if (str[i] == character) {
            return (1);
        }
        i = i + 1;
    }
    return (0);
}
```

---

## 6. Variables & Scopes

Variables are declared by specifying their type followed by their identifier.
They can optionally include an initial value statement.

```c
// Uninitialized declaration
Pos pos1;
pos1.col = 1;
pos1.line = 2;

// Declaration with initialization
i32 index = 0;

// Fixed-size array and pointer declarations
Token[2] tmp;
Token* test = &tmp[0];

```

---

## 7. Control Flow

`if` / `else` Statements.

```c
if (ret == 0) {
    write(1, "Pass\n", 5);
} else {
    write(1, "Fail\n", 5);
}
```

`while` Loops.

```c
while (tokens[index].type != 0) {
    index = index + 1;
}
```

---

## 8. Operators & Precedence

Operator precedence order, ordered from highest to lowest:

- **Postfix**: Function call `f()`, Array indexing `a[i]`, Member access `obj.prop`
- **Unary**: Logical NOT `!`, Unary minus `-`, Address-of `&`, Dereference `*`
- **Multiplicative**: `*`, `/`, `%`
- **Additive**: `+`, `-`
- **Relational**: `<`, `>`, `<=`, `>=`
- **Equality**: `==`, `!=`
- **Logical AND**: `&&`
- **Logical OR**: `||`
- **Assignment**: `=` _( Right-associative:_ `a = b = 5` _)_

---

## 9. System Calls (`syscall`)

The language provides direct low-level kernel interaction via the built-in `syscall(id, arg1, arg2, ...)` intrinsic:

```c
// Performs a sys_write call (syscall #1 on x86_64 Linux)
fun write(i32 fd, string msg, i64 len) i64 {
    return syscall(1, fd, msg, len);
}
```
