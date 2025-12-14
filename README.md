## bit(N) Compiler - Language Guide
![bit(N) Compiler - Language Guide](bitN.jpg)

## Status: Phase 2 In Progress 🚀

**Latest Update:** Syntax redesign complete. Nim-style indentation-based language with named operators. Semantic analysis framework integrated. Symbol table and type inference operational.

The bit(N) compiler now includes:
- ✅ Lexical analysis (tokenization) - Updated for new syntax
- ✅ Syntax analysis (parsing) - Indentation-based blocks
- ✅ AST construction
- ✅ Type system integration
- ✅ Semantic analysis (symbol table, type inference)

---

## Quick Start

### Installation

```bash
cd ~/bit-n
bash bitN_setup.sh
```

### First Program

```bash
./build/bitN -c 'proc main(): u32 = return 42'
```

### Expected Output

```
=== bit(N) Compiler ===
Input: command-line

--- Lexical Analysis ---
Token(PROC, line=1, col=0, len=4)
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

### 1. Functions (proc/func)

Define functions with explicit return types using `proc` or `func`:

```bitn
proc add(x: u32, y: u32): u32 =
  return add(x, y)

func multiply(x: u32, y: u32): u32 =
  return mul(x, y)

proc main(): u32 =
  let result: u32 = add(10, 20)
  return result
```

**Syntax:**
- `proc` or `func` keyword marks function definition
- Parameters with type annotations: `name: type`
- Return type after `:`: `: type`
- Body uses indentation (no braces)
- Colon at end of function signature

**Difference:**
- `proc` - Traditional procedure-style function
- `func` - Functional-style function (semantics identical currently)

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

Declare variables with `let` or `var`:

```bitn
proc example(): u32 =
  let x: u32 = 10
  var y: u32 = 20
  let result: u32 = add(x, y)
  return result
```

**Syntax:**
- `let name: type = value` - Immutable variable
- `var name: type = value` - Mutable variable (planned)
- Type annotation is required
- Values must match the declared type
- No semicolons required (newline-terminated)
- Scope tracked correctly in indented blocks

---

### 4. Named Arithmetic Operators

Instead of infix operators, use named functions:

```bitn
proc math(): u32 =
  let a: u32 = 10
  let b: u32 = 3
  
  let sum: u32 = add(a, b)           // Addition: 13
  let diff: u32 = sub(a, b)          // Subtraction: 7
  let product: u32 = mul(a, b)       // Multiplication: 30
  let quotient: u32 = div(a, b)      // Division: 3
  let remainder: u32 = mod(a, b)     // Modulo: 1
  
  return quotient
```

**Named Operators:**
- `add(x, y)` - Addition
- `sub(x, y)` - Subtraction
- `mul(x, y)` - Multiplication
- `div(x, y)` - Division (integer)
- `mod(x, y)` - Modulo (remainder)
- `neg(x)` - Negation

---

### 5. Named Bitwise Operators

Specialized bitwise operations as named functions:

```bitn
proc bitwise(): u32 =
  let a: u32 = 0xAA
  let b: u32 = 0x55
  
  let and_result: u32 = bitand(a, b)     // Bitwise AND
  let or_result: u32 = bitor(a, b)       // Bitwise OR
  let xor_result: u32 = bitxor(a, b)     // Bitwise XOR
  let left_shift: u32 = shl(a, 4)        // Left shift by 4
  let right_shift: u32 = shr(a, 2)       // Right shift by 2
  
  return xor_result
```

**Named Bitwise Operators:**
- `bitand(x, y)` - Bitwise AND
- `bitor(x, y)` - Bitwise OR
- `bitxor(x, y)` - Bitwise XOR
- `shl(x, n)` - Left shift by n
- `shr(x, n)` - Right shift by n
- `bitnot(x)` - Bitwise NOT (unary)

---

### 6. Named Comparison Operators

```bitn
proc compare(): u32 =
  let x: u32 = 10
  let y: u32 = 20
  
  let eq: u32 = eq(x, y)    // Equals: 0 (false)
  let ne: u32 = ne(x, y)    // Not equals: 1 (true)
  let lt: u32 = lt(x, y)    // Less than: 1 (true)
  let gt: u32 = gt(x, y)    // Greater than: 0 (false)
  let le: u32 = le(x, y)    // Less or equal: 1 (true)
  let ge: u32 = ge(x, y)    // Greater or equal: 0 (false)
  
  return ne
```

**Named Comparison Operators:**
- `eq(x, y)` - Equal
- `ne(x, y)` - Not equal
- `lt(x, y)` - Less than
- `gt(x, y)` - Greater than
- `le(x, y)` - Less or equal
- `ge(x, y)` - Greater or equal

---

### 7. Return Statements

```bitn
proc factorial(n: u32): u32 =
  if eq(n, 0):
    return 1
  return n
```

Return from a function with a value matching the return type.

---

### 8. Number Formats

```bitn
proc numbers(): u32 =
  let decimal: u32 = 255          // Decimal: base 10
  let hex: u32 = 0xFF             // Hexadecimal: base 16
  let binary: u32 = 0b11111111    // Binary: base 2
  
  return hex
```

**Formats:**
- `255` - Decimal (base 10)
- `0xFF` or `0Xff` - Hexadecimal (base 16)
- `0b11111111` or `0B11111111` - Binary (base 2)

---

### 9. Bit Slices (Planned Phase 3)

Extract ranges of bits from values:

```bitn
proc extract(x: u32): u8 =
  return x[8:16]  // Extract bits 8 through 15
```

**Syntax:**
- `value[start:end]` - Extract bits from start to end (exclusive)
- Returns type appropriate for bit width
- Supports all integer types

---

## Example Programs

### Example 1: Extract Bits

```bitn
proc extract_bits(): u32 =
  let value: u32 = 0xFF
  let mask: u32 = 0x0F
  let result: u32 = bitand(value, mask)
  return result
```

### Example 2: Bit Shifting

```bitn
proc shift_operations(): u32 =
  let value: u32 = 1
  let shifted_left: u32 = shl(value, 4)
  let shifted_right: u32 = shr(shifted_left, 2)
  return shifted_right
```

### Example 3: Multiple Bitwise Operations

```bitn
proc bit_operations(): u32 =
  let a: u32 = 0xAAAA
  let b: u32 = 0x5555
  
  let and_result: u32 = bitand(a, b)
  let or_result: u32 = bitor(a, b)
  let xor_result: u32 = bitxor(a, b)
  
  return xor_result
```

### Example 4: Bit Counting

```bitn
proc count_set_bits(): u32 =
  let value: u32 = 0xFFFF
  let shifted: u32 = shr(value, 8)
  let count: u32 = add(value, shifted)
  return count
```

---

## Usage

### Compile from Command Line

#### Inline Code
```bash
./build/bitN -c 'proc main(): u32 = return 42'
```

#### From File
```bash
./build/bitN examples/extract_bits.bitn
```

#### With Operations
```bash
./build/bitN -c 'proc test(): u32 = let x: u32 = add(10, 20); return x'
```

---

## What the Compiler Does

### 1. Lexical Analysis
- Tokenizes the input
- Recognizes keywords: `proc`, `func`, `let`, `var`, `return`, `if`, `while`
- Recognizes indentation tokens: `INDENT`, `DEDENT`
- Parses numbers: decimal, hex (0x...), binary (0b...)
- Identifies operators and delimiters
- Tracks line and column positions

### 2. Syntax Analysis
- Builds an Abstract Syntax Tree (AST)
- Validates function definitions (proc/func)
- Checks expression syntax
- Verifies statement structure
- Handles indentation-based blocks
- No braces required

### 3. Semantic Analysis
- Tracks variable declarations in scopes
- Infers expression types
- Validates type compatibility
- Detects undefined variables
- Prevents variable redefinition
- Validates return types

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
- `lexer.c` - Lexical analyzer (UPDATED: indentation tracking)
- `ast.c` - AST management
- `parser.c` - Parser implementation (UPDATED: proc/func/var support)
- `type_system.c` - Type operations
- `symbol_table.c` - Scope tracking
- `type_inference.c` - Type validation
- `main.c` - Compiler entry point

### Examples (`examples/`)
- `extract_bits.bitn` - Bit extraction (UPDATED: Nim-style syntax)
- `bit_manipulation.bitn` - Multiple operations (UPDATED)
- `count_bits.bitn` - Bit counting (UPDATED)

---

## Current Capabilities

### What Works ✅
- Parse any function definition (proc/func)
- Recognize all keywords and operators
- Handle all number formats (decimal, hex, binary)
- Support all named operator types
- Validate type annotations
- Check function return types
- Track variable scopes
- Detect undefined variables
- Infer expression types
- Report compilation phases clearly
- Full three-phase compilation pipeline
- Indentation-based block parsing
- Recognize `proc`, `func`, `var` keywords

### What's Coming 🔧
- Better error messages for type mismatches
- Advanced control flow handling
- Full variable scope refinement

### Future Enhancements 🚀
- Code generation (ARM, RISC-V, x86)
- Bitfield support
- Struct definitions
- Array support
- Pointer support
- Optimization passes
- Inline assembly

---

## Syntax Comparison

### Old Syntax (Rust-style)
```rust
fn add(x: u32, y: u32) -> u32 {
    return x + y;
}

fn main() -> u32 {
    let a: u32 = 10;
    let b: u32 = 20;
    return a + b;
}
```

### New Syntax (Nim-style) ✨
```bitn
proc add(x: u32, y: u32): u32 =
  return add(x, y)

proc main(): u32 =
  let a: u32 = 10
  let b: u32 = 20
  return add(a, b)
```

**Key Changes:**
- `fn` → `proc` or `func`
- `-> type` → `: type`
- `{ }` → indentation-based blocks
- Infix operators → named functions
- `x + y` → `add(x, y)`
- Semicolons optional (newline-terminated)

---

## Limitations (Current Version)

- **No code generation** - Parser to type checker only
- **Named operators only** - Infix syntax planned for Phase 3
- **Functions not callable** - Within code (planned)
- **No arrays or pointers** - Planned for Phase 3
- **No structs or custom types** - Planned for Phase 4
- **Bitfield support** - Planned for Phase 3
- **Bit slices not yet implemented** - Phase 3

---

## Troubleshooting

### Parse Error Messages
Check:
1. Function syntax: `proc name(): type = ...` or `func name(): type = ...`
2. Variable syntax: `let x: type = value`
3. Return statements: `return value`
4. Type annotations present on variables
5. Proper indentation (no tabs/spaces mix)

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
   ./build/bitN -c 'proc test(): u32 = return 42'
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

**Ready to compile bit(N) code in Nim-style!** 🚀
