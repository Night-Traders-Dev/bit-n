# bit(N) Compiler - Language Guide

A specialized compiler for the **bit(N)** language - a low-level language optimized for bitwise operations, embedded systems, and hardware manipulation.

## Quick Start

### Installation

```bash
cd ~/bit-n
bash rebuild-source.sh
```

### First Program

```bash
./build/bitN -c "fn main() -> u32 { return 42; }"
```

### Output

```
=== bit(N) Compiler ===
Input: -c

--- Lexical Analysis ---
Token(FN, line=1, col=0, len=2)
Token(IDENTIFIER, line=1, col=3, len=4)
Token(LPAREN((), line=1, col=7, len=1)
...
--- Parsing ---
Successfully parsed 1 functions
  - main

=== Compilation Complete ===
```

## Language Features

### 1. Functions

Define functions with explicit return types:

```bitn
fn add(x: u32, y: u32) -> u32 {
    return x + y;
}

fn main() -> u32 {
    return 42;
}
```

**Syntax:**
- `fn` keyword marks function definition
- Parameters with type annotations: `name: type`
- Return type after `->`: `-> type`
- Body in braces: `{ ... }`

### 2. Type System

#### Integer Types (Unsigned)
- `u8` - 8-bit unsigned integer
- `u16` - 16-bit unsigned integer
- `u32` - 32-bit unsigned integer (most common)
- `u64` - 64-bit unsigned integer

#### Integer Types (Signed)
- `i8` - 8-bit signed integer
- `i16` - 16-bit signed integer
- `i32` - 32-bit signed integer
- `i64` - 64-bit signed integer

#### Special Types
- `void` - No return value

### 3. Variables

Declare variables with `let`:

```bitn
fn example() -> u32 {
    let x: u32 = 10;
    let y: u32 = 20;
    let result: u32 = x + y;
    return result;
}
```

**Syntax:**
- `let name: type = value;`
- Type annotation is required
- Values must match the declared type

### 4. Arithmetic Operators

```bitn
fn math() -> u32 {
    let a: u32 = 10;
    let b: u32 = 3;
    
    let sum: u32 = a + b;        // Addition: 13
    let diff: u32 = a - b;       // Subtraction: 7
    let product: u32 = a * b;    // Multiplication: 30
    let quotient: u32 = a / b;   // Division: 3
    let remainder: u32 = a % b;  // Modulo: 1
    
    return quotient;
}
```

**Operators:**
- `+` Addition
- `-` Subtraction
- `*` Multiplication
- `/` Division (integer)
- `%` Modulo (remainder)

### 5. Bitwise Operators

Specialized for bit manipulation:

```bitn
fn bitwise() -> u32 {
    let a: u32 = 0xAA;
    let b: u32 = 0x55;
    
    let and_result: u32 = a & b;      // Bitwise AND
    let or_result: u32 = a | b;       // Bitwise OR
    let xor_result: u32 = a ^ b;      // Bitwise XOR
    
    let left_shift: u32 = a << 4;     // Left shift by 4
    let right_shift: u32 = a >> 2;    // Right shift by 2
    
    return xor_result;
}
```

**Operators:**
- `&` Bitwise AND
- `|` Bitwise OR
- `^` Bitwise XOR
- `<<` Left shift
- `>>` Right shift
- `~` Bitwise NOT (unary)

### 6. Comparison Operators

```bitn
fn compare() -> u32 {
    let x: u32 = 10;
    let y: u32 = 20;
    
    let eq: u32 = x == y;    // Equals: 0 (false)
    let ne: u32 = x != y;    // Not equals: 1 (true)
    let lt: u32 = x < y;     // Less than: 1 (true)
    let gt: u32 = x > y;     // Greater than: 0 (false)
    let le: u32 = x <= y;    // Less or equal: 1 (true)
    let ge: u32 = x >= y;    // Greater or equal: 0 (false)
    
    return ne;
}
```

**Operators:**
- `==` Equal
- `!=` Not equal
- `<` Less than
- `>` Greater than
- `<=` Less or equal
- `>=` Greater or equal

### 7. Return Statements

Return from a function:

```bitn
fn factorial(n: u32) -> u32 {
    if n == 0 {
        return 1;
    }
    return n;
}
```

### 8. Number Formats

Support for multiple number bases:

```bitn
fn numbers() -> u32 {
    let decimal: u32 = 255;       // Decimal
    let hex: u32 = 0xFF;          // Hexadecimal
    let binary: u32 = 0b11111111; // Binary
    
    return hex;
}
```

**Formats:**
- `255` - Decimal (base 10)
- `0xFF` - Hexadecimal (base 16)
- `0b11111111` - Binary (base 2)

## Example Programs

### Example 1: Extract Bits

```bitn
fn extract_bits() -> u32 {
    let value: u32 = 0xFF;
    let mask: u32 = 0x0F;
    let result: u32 = value & mask;
    return result;
}
```

### Example 2: Bit Shifting

```bitn
fn shift_operations() -> u32 {
    let value: u32 = 1;
    let shifted_left: u32 = value << 4;
    let shifted_right: u32 = shifted_left >> 2;
    return shifted_right;
}
```

### Example 3: Multiple Operations

```bitn
fn bit_operations() -> u32 {
    let a: u32 = 0xAAAA;
    let b: u32 = 0x5555;
    
    let and_result: u32 = a & b;
    let or_result: u32 = a | b;
    let xor_result: u32 = a ^ b;
    
    return xor_result;
}
```

## Usage

### Compile from Command Line

```bash
# Inline code
./build/bitN -c "fn main() -> u32 { return 42; }"

# From file
./build/bitN examples/extract_bits.bitn

# With operations
./build/bitN -c "fn test() -> u32 { let x: u32 = 10 + 20; return x; }"
```

### What the Compiler Does

1. **Lexical Analysis** - Tokenizes the input
   - Recognizes keywords: `fn`, `let`, `return`, `if`, `while`
   - Parses numbers (decimal, hex, binary)
   - Identifies operators and delimiters

2. **Syntax Analysis** - Builds an Abstract Syntax Tree (AST)
   - Validates function definitions
   - Checks expression syntax
   - Verifies statement structure

3. **Output** - Displays:
   - All tokens found
   - Successfully parsed functions
   - Errors (if any)

## File Organization

```
~/bit-n/
├── CMakeLists.txt
├── include/
│   ├── token.h      (Token types)
│   ├── lexer.h      (Lexer interface)
│   ├── ast.h        (AST definitions)
│   └── parser.h     (Parser interface)
├── src/
│   ├── token.c      (Token utilities)
│   ├── lexer.c      (Lexical analyzer)
│   ├── ast.c        (AST management)
│   ├── parser.c     (Parser implementation)
│   └── main.c       (Compiler entry point)
├── build/
│   └── bitN         (Compiled executable)
└── examples/
    ├── extract_bits.bitn
    ├── bit_manipulation.bitn
    └── count_bits.bitn
```

## Project Structure

### Headers (include/)

- **token.h** - Token type definitions and printing
- **lexer.h** - Lexer interface for tokenization
- **ast.h** - AST node structures for all language constructs
- **parser.h** - Parser interface for syntax analysis

### Sources (src/)

- **token.c** - Token utility functions
- **lexer.c** - Lexical analyzer with keyword recognition
- **ast.c** - AST node creation and memory management
- **parser.c** - Recursive descent parser with error recovery
- **main.c** - Compiler entry point and command-line handling

### Build System

- **CMakeLists.txt** - CMake configuration for building

### Examples

- **examples/** - Sample bit(N) programs to compile

## Common Patterns

### Basic Function

```bitn
fn compute() -> u32 {
    return 100;
}
```

### With Local Variables

```bitn
fn calculate() -> u32 {
    let x: u32 = 10;
    let y: u32 = 20;
    let z: u32 = x + y;
    return z;
}
```

### Bitwise Manipulation

```bitn
fn extract() -> u32 {
    let data: u32 = 0xDEADBEEF;
    let mask: u32 = 0xFF;
    let lowest: u32 = data & mask;
    return lowest;
}
```

### Multiple Functions

```bitn
fn helper() -> u32 {
    return 42;
}

fn main() -> u32 {
    return helper();
}
```

## Limitations (Current Version)

- ⚠️ No code generation yet (parser only)
- ⚠️ Functions not callable within code
- ⚠️ No arrays or pointers
- ⚠️ No structs or custom types
- ⚠️ No if/while statements (parsed but not executed)
- ⚠️ No comments in output

## Future Enhancements

- ✅ Code generation (ARM, RISC-V, x86)
- ✅ Type checking and validation
- ✅ Control flow (if/else, loops)
- ✅ Function calls
- ✅ Arrays and pointers
- ✅ Struct definitions
- ✅ Optimization passes
- ✅ Debug symbols

## Troubleshooting

### "Expected function definition" Error

**Problem:** Keywords like `fn` are tokenized as `IDENTIFIER`

**Solution:** Rebuild with fixed lexer
```bash
bash rebuild-source.sh
```

### Compile Errors

**Check:**
1. Function syntax: `fn name() -> type { ... }`
2. Variable syntax: `let x: type = value;`
3. Return statements: `return value;`

### Build Failures

**Solution:** Clean rebuild
```bash
cd ~/bit-n/build
rm -rf *
cmake ..
make
```

## Performance

- **Compilation time:** < 100ms typical
- **Memory usage:** < 10MB typical
- **Token count:** Scales linearly with source size

## License

Open source - use freely for learning and embedded systems

## Support

For issues with:
- **Compilation**: Check CMakeLists.txt is in build/
- **Lexing**: Verify fn/let/return are recognized as FN/LET/RETURN tokens
- **Parsing**: Check function definition syntax is correct

---

**Ready to compile bit(N) code! 🚀**

For implementation details, see **IMPLEMENTATION.md**
