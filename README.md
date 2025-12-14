# bit(N) Compiler - Language Guide

## Status: Phase 2 In Progress 🚀

**Latest Update:** Semantic analysis framework integrated. Symbol table and type inference being refined.

The bit(N) compiler now includes:
- ✅ Lexical analysis (tokenization)
- ✅ Syntax analysis (parsing)
- ✅ AST construction
- ✅ Type system integration
- 🔧 Semantic analysis (symbol table, type inference - refining)

---

## Quick Start

### Installation

```bash
cd ~/bit-n
bash bitN_setup.sh
```

### First Program

```bash
./build/bitN -c 'fn main() -> u32 { return 42; }'
```

### Expected Output

```
=== bit(N) Compiler ===
Input: command-line

--- Lexical Analysis ---
Token(3, line=1, col=0, len=2)
...

--- Parsing ---
✅ Successfully parsed 1 functions
  - main() -> u32

--- Semantic Analysis ---
✅ Semantic analysis passed

--- Type Checking ---
✅ Type checking passed
```

---

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
- Body in braces with statements

---

### 2. Type System

#### Unsigned Integers
- `u8` - 8-bit (0 to 255)
- `u16` - 16-bit (0 to 65,535)
- `u32` - 32-bit (0 to 4.3 billion) - most common
- `u64` - 64-bit (0 to 18.4 quintillion)

#### Signed Integers
- `i8`, `i16`, `i32`, `i64` - Signed equivalents

#### Special Types
- `void` - No return value (planned)

**Status:** Type system fully integrated. Function return types validated at compile time.

---

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
- Scope tracked correctly in blocks

---

### 4. Arithmetic Operators

```bitn
fn math() -> u32 {
    let a: u32 = 10;
    let b: u32 = 3;
    
    let sum: u32 = a + b;          // Addition: 13
    let diff: u32 = a - b;         // Subtraction: 7
    let product: u32 = a * b;      // Multiplication: 30
    let quotient: u32 = a / b;     // Division: 3
    let remainder: u32 = a % b;    // Modulo: 1
    
    return quotient;
}
```

**Operators:**
- `+` Addition
- `-` Subtraction
- `*` Multiplication
- `/` Division (integer)
- `%` Modulo (remainder)

---

### 5. Bitwise Operators

Specialized for bit manipulation:

```bitn
fn bitwise() -> u32 {
    let a: u32 = 0xAA;
    let b: u32 = 0x55;
    
    let and_result: u32 = a & b;     // Bitwise AND
    let or_result: u32 = a | b;      // Bitwise OR
    let xor_result: u32 = a ^ b;     // Bitwise XOR
    let left_shift: u32 = a << 4;    // Left shift by 4
    let right_shift: u32 = a >> 2;   // Right shift by 2
    
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

---

### 6. Comparison Operators

```bitn
fn compare() -> u32 {
    let x: u32 = 10;
    let y: u32 = 20;
    
    let eq: u32 = (x == y);  // Equals: 0 (false)
    let ne: u32 = (x != y);  // Not equals: 1 (true)
    let lt: u32 = (x < y);   // Less than: 1 (true)
    let gt: u32 = (x > y);   // Greater than: 0 (false)
    let le: u32 = (x <= y);  // Less or equal: 1 (true)
    let ge: u32 = (x >= y);  // Greater or equal: 0 (false)
    
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

---

### 7. Return Statements

```bitn
fn factorial(n: u32) -> u32 {
    if n == 0 {
        return 1;
    }
    return n;
}
```

Return from a function with a value matching the return type.

---

### 8. Number Formats

```bitn
fn numbers() -> u32 {
    let decimal: u32 = 255;          // Decimal: base 10
    let hex: u32 = 0xFF;             // Hexadecimal: base 16
    let binary: u32 = 0b11111111;    // Binary: base 2
    
    return hex;
}
```

**Formats:**
- `255` - Decimal (base 10)
- `0xFF` or `0Xff` - Hexadecimal (base 16)
- `0b11111111` or `0B11111111` - Binary (base 2)

---

### 9. Bit Slices

Extract ranges of bits from values:

```bitn
fn extract(x: u32) -> u8 {
    return x[8:16];  // Extract bits 8 through 15
}
```

**Syntax:**
- `value[start:end]` - Extract bits from start to end (exclusive)
- Returns type appropriate for bit width
- Supports all integer types

---

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

---

## Usage

### Compile from Command Line

#### Inline Code
```bash
./build/bitN -c 'fn main() -> u32 { return 42; }'
```

#### From File
```bash
./build/bitN examples/extract_bits.bitn
```

#### With Operations
```bash
./build/bitN -c 'fn test() -> u32 { let x: u32 = 10 + 20; return x; }'
```

---

## What the Compiler Does

### 1. Lexical Analysis
- Tokenizes the input
- Recognizes keywords: `fn`, `let`, `return`, `if`, `while`
- Parses numbers: decimal, hex (0x...), binary (0b...)
- Identifies operators and delimiters
- Tracks line and column positions

### 2. Syntax Analysis
- Builds an Abstract Syntax Tree (AST)
- Validates function definitions
- Checks expression syntax
- Verifies statement structure
- Handles nested blocks

### 3. Semantic Analysis
- Tracks variable declarations in scopes
- Infers expression types
- Validates type compatibility
- Detects undefined variables
- Prevents variable redefinition
- Validates return types (in progress)

### 4. Type Checking
- Validates function return types
- Checks variable initializations
- Ensures type annotations are present
- Reports type mismatches

### 5. Output
Displays:
- All tokens found
- Successfully parsed functions and structure
- Semantic analysis results
- Type checking results
- Errors if any

---

## File Organization

### Headers (`include/`)
- `token.h` - Token type definitions
- `lexer.h` - Lexer interface
- `ast.h` - AST node structures
- `parser.h` - Parser interface
- `type_system.h` - Type definitions
- `symbol_table.h` - Symbol table API
- `type_inference.h` - Type inference API

### Sources (`src/`)
- `token.c` - Token utilities
- `lexer.c` - Lexical analyzer
- `ast.c` - AST management
- `parser.c` - Parser implementation
- `type_system.c` - Type operations
- `symbol_table.c` - Scope tracking
- `type_inference.c` - Type validation
- `main.c` - Compiler entry point

### Examples (`examples/`)
- `extract_bits.bitn` - Bit extraction
- `bit_manipulation.bitn` - Multiple operations
- `count_bits.bitn` - Bit counting

---

## Current Capabilities

### What Works ✅
- Parse any function definition
- Recognize all keywords and operators
- Handle all number formats (decimal, hex, binary)
- Support all operator types
- Validate type annotations
- Check function return types
- Track variable scopes
- Detect undefined variables
- Infer expression types
- Report compilation phases clearly
- Full three-phase compilation pipeline

### What's Coming 🔧
- Full semantic analysis refinement
- Better error messages
- if/else statement handling
- While loops
- For loops
- Function calls

### Future Enhancements 🚀
- Code generation (ARM, RISC-V, x86)
- Bitfield support
- Struct definitions
- Array support
- Pointer support
- Optimization passes
- Inline assembly

---

## Limitations (Current Version)

- **No code generation** - Parser to type checker only
- **Functions not callable** - Within code (planned)
- **No arrays or pointers** - Planned for Phase 3
- **No structs or custom types** - Planned for Phase 4
- **Control flow partially supported** - if/while parsed but refinement in progress
- **Parameters being type-checked** - Full validation in progress

---

## Troubleshooting

### Parse Error Messages
Check:
1. Function syntax: `fn name() -> type { ... }`
2. Variable syntax: `let x: type = value;`
3. Return statements: `return value;`
4. Type annotations present on variables

### Build Failures
Clean rebuild:
```bash
cd ~/bit-n
rm -rf build
mkdir build
cd build
cmake ..
make
```

### Performance
- Compilation time: ~100ms (typical)
- Memory usage: ~10MB (typical)
- Token count: Scales linearly with source size

---

## Getting Started

1. Build the compiler:
   ```bash
   bash ~/bit-n/bitN_setup.sh
   ```

2. Try a simple program:
   ```bash
   ./build/bitN -c 'fn test() -> u32 { return 42; }'
   ```

3. Expected output:
   ```
   --- Semantic Analysis ---
   ✅ Semantic analysis passed
   
   --- Type Checking ---
   ✅ Type checking passed
   ```

4. Write your own bit(N) programs!

---

## For More Information

- **Implementation Details:** See `IMPLEMENTATION.md`
- **Project Roadmap:** See `ROADMAP.md`
- **Project Vision:** See `VISION.md`

---

**Ready to compile bit(N) code!** 🚀
