# bit(N) Compiler - Implementation Details

## Current Status: Phase 2 Complete ✅ | Code Generation Active 🚀

**Latest:** Device definition parsing complete. C header code generation operational. Full compilation pipeline from `.bitn` device files to `.h` headers with register structures and bit field accessors.

---

## Compiler Architecture

```
Source Code (.bitn) - Device Definitions
    ↓
PHASE 1: Lexical Analysis (✅ Complete)
    ├─ Tokenization with indentation
    ├─ Keyword recognition (peripheral, register, field)
    ├─ INDENT/DEDENT token generation
    └─ Position tracking
    ↓ Tokens
PHASE 1: Syntax Analysis (✅ Complete)
    ├─ Recursive descent parsing
    ├─ Device definition parsing
    ├─ Register and field parsing
    ├─ AST construction
    └─ Error recovery
    ↓ Abstract Syntax Tree
PHASE 2: Semantic Analysis (✅ Complete)
    ├─ Symbol table (✅ Done)
    ├─ Type inference (✅ Done)
    ├─ Scope tracking (✅ Done)
    └─ Error detection (✅ Done)
    ↓ Validated AST
PHASE 3: Code Generation (✅ Operational)
    ├─ C header generation (✅ Done)
    ├─ Register struct creation
    ├─ Bit field macro generation
    ├─ Inline accessor functions
    └─ Device memory mapping
    ↓ Generated C Headers (.h)
```

---

## Device Definition Support

### Peripheral Definitions

```bitn
peripheral UART {
    base_address: 0x40000000
    
    register CONTROL {
        type: u32
        offset: 0x0
        
        field ENABLE {
            start_bit: 0
            end_bit: 1
            access: rw
        }
    }
}
```

**Parsed Elements:**
- `peripheral` keyword - Device definition start
- `base_address` - Memory-mapped address
- `register` - Register within peripheral
- `type` - Register bit width (u8, u16, u32, u64)
- `offset` - Register offset from base address
- `field` - Bit field within register
- `access` - Access type (ro, wo, rw, w1c)

### Semantic Analysis for Devices

The semantic analyzer validates:
- Register offsets don't overlap
- Fields fit within register width
- Bit ranges are valid (start_bit < end_bit)
- Access types are recognized
- Base addresses are valid

---

## Code Generation Pipeline

### Input Processing

```c
// File: device.bitn
peripheral GPIO {
    base_address: 0x50000000
    register DIR {
        type: u32
        offset: 0x0
        field OUTPUT { start_bit: 0, end_bit: 32, access: rw }
    }
}
```

### Generated Output

```c
// File: device.h
#define GPIO ((volatile GPIO_t *)0x50000000)

typedef struct {
    uint32_t DIR;  // @ offset 0x0
} GPIO_t;

// Bit field macros
#define DIR_OUTPUT_GET(reg)      (((reg) >> 0) & 0xffffffff)
#define DIR_OUTPUT_SET(reg, val) (((reg) & ~(0xffffffff << 0)) | ((val & 0xffffffff) << 0))

// Inline accessors
static inline uint32_t DIR_OUTPUT_read(uint32_t reg) {
    return (reg >> 0) & 0xffffffff;
}

static inline uint32_t DIR_OUTPUT_write(uint32_t reg, uint32_t val) {
    return (reg & ~(0xffffffff << 0)) | ((val & 0xffffffff) << 0);
}
```

### Generation Features

✅ **Register Structures**
- Memory-mapped pointers with volatile
- Correct register layout
- Offset tracking

✅ **Bit Field Macros**
- GET macros for reading bits
- SET macros for modifying bits
- Correct mask generation
- Bit shift calculation

✅ **Inline Functions**
- Read accessors (safe extraction)
- Write accessors (safe modification)
- No memory overhead
- Inlined by compiler

✅ **Smart Filenames**
- Input: `uart.bitn` → Output: `uart.h`
- Input: `gpio_device.bitn` → Output: `gpio_device.h`
- Automatic extension replacement

---

## Main Entry Point

The updated `main.c` provides:

```c
int main(int argc, char *argv[])
```

**Arguments Supported:**
- `bitN device.bitn` - Parse and display
- `bitN --compile device.bitn` - Generate C headers
- `bitN --compile device.bitn --verbose` - With detailed output
- `bitN -c 'code'` - Inline device code

**Compilation Phases:**

```
1. ARGUMENT PARSING
   ├─ Detect --compile flag
   ├─ Detect --verbose flag
   └─ Locate input file

2. FILE LOADING
   ├─ Read .bitn file
   ├─ Allocate memory
   └─ Validate file contents

3. LEXICAL ANALYSIS
   ├─ Tokenize with indentation
   ├─ Optional verbose output (--verbose)
   └─ Generate tokens

4. PARSING
   ├─ Build AST from tokens
   ├─ Validate structure
   └─ Detect syntax errors

5. STRUCTURE DISPLAY
   ├─ Show functions
   ├─ Show peripherals with addresses
   ├─ Show registers with offsets
   └─ Show fields with bit ranges

6. CODE GENERATION (if --compile)
   ├─ Check for peripherals
   ├─ Initialize code generator
   ├─ Generate C headers
   ├─ Write output file
   └─ Report success/failure

7. CLEANUP
   ├─ Free AST
   ├─ Free parser
   ├─ Free lexer
   └─ Free allocated memory
```

---

## Backend Integration

### Code Generator Interface

```c
// Initialize code generation context
CodegenContext *ctx = codegen_init(output_file, "arm-cortex-m0");

// Generate C code from AST
int result = codegen_generate(ctx, program);

// Cleanup
codegen_cleanup(ctx);
```

### File Generation

The codegen backend:
- ✅ Opens output file for writing
- ✅ Generates includes and guards
- ✅ Creates struct definitions
- ✅ Generates accessor macros
- ✅ Generates inline functions
- ✅ Closes file properly
- ✅ Returns status code

### Error Handling

```c
if (!ctx) {
    fprintf(stderr, "Error: Failed to initialize code generator\n");
    return 1;
}

if (codegen_generate(ctx, program) != 0) {
    fprintf(stderr, "❌ Code generation failed\n");
    codegen_cleanup(ctx);
    return 1;
}

printf("✅ Successfully generated C code\n");
```

---

## File Statistics

| Component | Status | Lines |
|-----------|--------|-------|
| main.c | ✅ Complete | 229 |
| lexer.c | ✅ Complete | 400 |
| parser.c | ✅ Complete | 500+ |
| ast.c | ✅ Complete | 250 |
| type_system.c | ✅ Complete | 170 |
| symbol_table.c | ✅ Complete | 200 |
| type_inference.c | ✅ Complete | 350 |
| codegen.c | ✅ Complete | 200+ |
| **Total** | **✅** | **~2,400** |

---

## Device Definition Grammar

```
Program
  └─ Function* | Peripheral*

Peripheral
  ├─ "peripheral" IDENTIFIER
  ├─ "base_address" ":" HEX_NUMBER
  └─ Register*

Register
  ├─ "register" IDENTIFIER
  ├─ "type" ":" TYPE
  ├─ "offset" ":" HEX_NUMBER
  └─ Field*

Field
  ├─ "field" IDENTIFIER
  ├─ "start_bit" ":" NUMBER
  ├─ "end_bit" ":" NUMBER
  └─ "access" ":" ACCESS_TYPE

TYPE: u8 | u16 | u32 | u64
ACCESS_TYPE: ro | wo | rw | w1c
```

---

## Usage Examples

### Parse Device Definition

```bash
./build/bitN uart.bitn
```

**Output:**
```
✅ Successfully parsed
 Peripherals: 1
 - peripheral UART @ 0x40000000
 * register CONTROL: u32 @ offset 0x00
 - field ENABLE: [0:1] rw
 - field BAUDRATE: [1:16] rw
 * register STATUS: u32 @ offset 0x04
 - field TX_READY: [0:1] ro
```

### Generate C Headers

```bash
./build/bitN --compile uart.bitn
cat uart.h
```

**Output:**
```c
#define UART ((volatile UART_t *)0x40000000)

typedef struct {
    uint32_t CONTROL;  // @ offset 0x0
    uint32_t STATUS;   // @ offset 0x4
} UART_t;

#define CONTROL_ENABLE_GET(reg)      (((reg) >> 0) & 0x1)
#define CONTROL_ENABLE_SET(reg, val) (((reg) & ~(0x1 << 0)) | ((val & 0x1) << 0))
```

### Verbose Generation

```bash
./build/bitN --compile uart.bitn --verbose
```

Shows tokens, parsing details, and generation progress.

---

## Integration with Projects

Generated headers can be used directly:

```c
#include "uart.h"

int main() {
    // Get UART peripheral
    volatile UART_t *uart = UART;
    
    // Read status
    uint32_t status = uart->STATUS;
    
    // Check if TX ready
    if (STATUS_TX_READY_GET(status)) {
        // Send data
        uart->CONTROL = CONTROL_ENABLE_SET(uart->CONTROL, 1);
    }
    
    return 0;
}
```

---

## Features

✅ **Complete Device Parsing**
- Peripheral definitions with memory addresses
- Register definitions with types and offsets
- Field definitions with bit ranges
- Access type specification

✅ **Robust Generation**
- Correct struct layouts
- Proper bit field macros
- Safe inline accessors
- Memory-mapped pointers

✅ **Error Handling**
- Clear error messages
- Line/column reporting
- Graceful failure

✅ **Command-line Options**
- `--compile` for C generation
- `--verbose` for detailed output
- Automatic filename generation
- File input support

---

## Implementation Status

### Complete ✅
- Device definition parsing
- Code generation backend
- C header output
- Struct generation
- Bit field macros
- Inline accessors
- Error handling
- Main entry point

### Next Steps 📅
- Advanced bitfield types
- Additional access patterns
- Code optimization
- Additional platform support

---

## For More Information

- **Language Features:** See `README.md`
- **Development Roadmap:** See `ROADMAP.md`
- **Strategic Vision:** See `VISION.md`
