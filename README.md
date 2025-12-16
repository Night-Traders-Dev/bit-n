# bit(N) Compiler - Device Definition & Code Generation Guide

![bit(N) Logo](assets/bit(N).jpg)

## Status: Device DSL Complete 🚀 | C Code Generation Operational ✅

**Latest Update:** Device definition language fully operational. C header generation from `.bitn` device files produces register structures, bit field macros, and inline accessor functions.

---

## Quick Start

### Installation

```bash
cd ~/bit-n
bash bitN_setup.sh
```

### Generate C Headers

```bash
./build/bitN --compile device.bitn
```

Creates: `device.h` with register structures and bit field accessors

---

## Device Definition Language

Define hardware peripherals with registers and bitfields:

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
        
        field BAUDRATE {
            start_bit: 1
            end_bit: 16
            access: rw
        }
    }
    
    register STATUS {
        type: u32
        offset: 0x4
        
        field TX_READY {
            start_bit: 0
            end_bit: 1
            access: ro
        }
        
        field RX_READY {
            start_bit: 1
            end_bit: 2
            access: ro
        }
    }
}
```

---

## Generated C Code

### Register Structure

```c
#define UART ((volatile UART_t *)0x40000000)

typedef struct {
    uint32_t CONTROL;  // @ offset 0x0
    uint32_t STATUS;   // @ offset 0x4
} UART_t;
```

### Bit Field Macros

```c
// Read operations
#define CONTROL_ENABLE_GET(reg)      (((reg) >> 0) & 0x1)
#define CONTROL_BAUDRATE_GET(reg)    (((reg) >> 1) & 0x7fff)
#define STATUS_TX_READY_GET(reg)     (((reg) >> 0) & 0x1)

// Modify operations
#define CONTROL_ENABLE_SET(reg, val) (((reg) & ~(0x1 << 0)) | ((val & 0x1) << 0))
#define CONTROL_BAUDRATE_SET(reg, val) (((reg) & ~(0x7fff << 1)) | ((val & 0x7fff) << 1))
```

### Inline Accessor Functions

```c
static inline uint32_t CONTROL_ENABLE_read(uint32_t reg) {
    return (reg >> 0) & 0x1;
}

static inline uint32_t CONTROL_ENABLE_write(uint32_t reg, uint32_t val) {
    return (reg & ~(0x1 << 0)) | ((val & 0x1) << 0);
}

static inline uint32_t CONTROL_BAUDRATE_read(uint32_t reg) {
    return (reg >> 1) & 0x7fff;
}

static inline uint32_t CONTROL_BAUDRATE_write(uint32_t reg, uint32_t val) {
    return (reg & ~(0x7fff << 1)) | ((val & 0x7fff) << 1);
}
```

---

## Language Features

### 1. Peripherals

Define a hardware peripheral with memory-mapped base address:

```bitn
peripheral NAME {
    base_address: 0xADDRESS
    ...
}
```

**Example:**
```bitn
peripheral GPIO {
    base_address: 0x50000000
    ...
}

peripheral I2C {
    base_address: 0x60000000
    ...
}
```

### 2. Registers

Define registers within a peripheral:

```bitn
register NAME {
    type: u32        # u8, u16, u32, or u64
    offset: 0x0      # Offset from base_address
    ...
}
```

**Example:**
```bitn
register CONTROL {
    type: u32
    offset: 0x0
}

register STATUS {
    type: u32
    offset: 0x4
}

register DATA {
    type: u16
    offset: 0x8
}
```

### 3. Bitfields

Define individual bit fields within registers:

```bitn
field NAME {
    start_bit: 0      # Starting bit (inclusive)
    end_bit: 8        # Ending bit (exclusive)
    access: rw        # ro, wo, rw, w1c
}
```

**Example:**
```bitn
field ENABLE {
    start_bit: 0
    end_bit: 1
    access: rw
}

field BAUDRATE {
    start_bit: 1
    end_bit: 16
    access: rw
}

field ERROR {
    start_bit: 16
    end_bit: 32
    access: ro
}
```

### 4. Access Types

- **ro** - Read-only
- **wo** - Write-only
- **rw** - Read-write
- **w1c** - Write-1-to-clear

---

## Usage Examples

### Example 1: UART Device

```bitn
peripheral UART {
    base_address: 0x40000000
    
    register CONTROL {
        type: u32
        offset: 0x0
        
        field ENABLE { start_bit: 0, end_bit: 1, access: rw }
        field TX_EN { start_bit: 1, end_bit: 2, access: rw }
        field RX_EN { start_bit: 2, end_bit: 3, access: rw }
    }
    
    register STATUS {
        type: u32
        offset: 0x4
        
        field TX_READY { start_bit: 0, end_bit: 1, access: ro }
        field RX_READY { start_bit: 1, end_bit: 2, access: ro }
    }
    
    register DATA {
        type: u8
        offset: 0x8
    }
}
```

### Example 2: GPIO Device

```bitn
peripheral GPIO {
    base_address: 0x50000000
    
    register DIR {
        type: u32
        offset: 0x0
        
        field OUTPUT {
            start_bit: 0
            end_bit: 32
            access: rw
        }
    }
    
    register OUT {
        type: u32
        offset: 0x4
        
        field PIN_STATE {
            start_bit: 0
            end_bit: 32
            access: rw
        }
    }
    
    register IN {
        type: u32
        offset: 0x8
        
        field PIN_STATE {
            start_bit: 0
            end_bit: 32
            access: ro
        }
    }
}
```

### Example 3: Multiple Devices

```bitn
peripheral TIMER {
    base_address: 0x40001000
    
    register COUNTER {
        type: u32
        offset: 0x0
        
        field VALUE {
            start_bit: 0
            end_bit: 32
            access: rw
        }
    }
}

peripheral I2C {
    base_address: 0x40002000
    
    register CONTROL {
        type: u8
        offset: 0x0
        
        field START { start_bit: 0, end_bit: 1, access: wo }
        field STOP { start_bit: 1, end_bit: 2, access: wo }
        field ACK { start_bit: 2, end_bit: 3, access: rw }
    }
}
```

---

## Compilation Workflow

### Step 1: Write Device Definition

```bash
cat > my_device.bitn << 'EOF'
peripheral UART {
    base_address: 0x40000000
    register CONTROL {
        type: u32
        offset: 0x0
        field ENABLE { start_bit: 0, end_bit: 1, access: rw }
    }
}
EOF
```

### Step 2: Generate C Headers

```bash
./build/bitN --compile my_device.bitn
```

### Step 3: Use in Your Code

```c
#include "my_device.h"

int main() {
    volatile UART_t *uart = UART;
    
    // Enable UART
    uart->CONTROL = CONTROL_ENABLE_SET(uart->CONTROL, 1);
    
    // Check if enabled
    if (CONTROL_ENABLE_GET(uart->CONTROL)) {
        // UART is enabled
    }
    
    return 0;
}
```

---

## Compiler Options

```bash
./build/bitN device.bitn
```
Parse and display device structure

```bash
./build/bitN --compile device.bitn
```
Generate C headers (creates device.h)

```bash
./build/bitN --compile device.bitn --verbose
```
Generate with detailed output

```bash
./build/bitN -c 'peripheral TEST { base_address: 0x1000 }'
```
Parse inline device definition

---

## Output Files

### Input: `uart.bitn`
**File Contents:**
```bitn
peripheral UART { ... }
```

### Output: `uart.h`
**Generated C header with:**
- Memory-mapped structure
- Base address macro
- Register definitions
- Bit field getter macros
- Bit field setter macros
- Inline accessor functions

---

## Generated Accessor Functions

For each bitfield, the compiler generates:

**Read Function:**
```c
static inline TYPE REGISTER_FIELD_read(uint32_t reg) {
    return (reg >> start_bit) & MASK;
}
```

**Write Function:**
```c
static inline uint32_t REGISTER_FIELD_write(uint32_t reg, TYPE val) {
    return (reg & ~(MASK << start_bit)) | ((val & MASK) << start_bit);
}
```

These compile to optimal inline code with zero overhead.

---

## Type System

Supported register types:
- `u8` - 8-bit unsigned
- `u16` - 16-bit unsigned
- `u32` - 32-bit unsigned
- `u64` - 64-bit unsigned

Bitfield types are automatically inferred from bit ranges.

---

## Best Practices

### 1. Clear Naming

```bitn
# ✅ Good
field TX_READY { start_bit: 0, end_bit: 1, access: ro }
field RX_ERROR { start_bit: 8, end_bit: 16, access: ro }

# ❌ Avoid
field F1 { start_bit: 0, end_bit: 1, access: ro }
field F2 { start_bit: 8, end_bit: 16, access: ro }
```

### 2. Bit Range Organization

```bitn
# ✅ Organize by function
field ENABLE { start_bit: 0, end_bit: 1, access: rw }
field MODE { start_bit: 1, end_bit: 4, access: rw }
field STATUS { start_bit: 4, end_bit: 8, access: ro }

# ❌ Scattered layout
field ENABLE { start_bit: 15, end_bit: 16, access: rw }
field MODE { start_bit: 0, end_bit: 3, access: rw }
```

### 3. Correct Access Types

```bitn
# ✅ Accurate access control
field CONTROL { start_bit: 0, end_bit: 8, access: rw }
field STATUS { start_bit: 8, end_bit: 16, access: ro }
field CLEAR { start_bit: 16, end_bit: 32, access: w1c }

# ❌ Overly permissive
field STATUS { start_bit: 8, end_bit: 16, access: rw }  # Should be ro
```

---

## Troubleshooting

### "No peripherals found to generate code for"

This means your `.bitn` file contains functions instead of peripheral definitions.

**Wrong:**
```bitn
proc main(): u32 = return 42
```

**Correct:**
```bitn
peripheral UART {
    base_address: 0x40000000
    ...
}
```

### Generated file is empty

1. Check file permissions
2. Ensure --compile flag is used
3. Verify peripheral definitions exist
4. Check for parse errors in output

### Bit field ordering issues

Ensure bit ranges don't overlap and are within register width:

```bitn
# ✅ Valid
field FIELD1 { start_bit: 0, end_bit: 8, access: rw }
field FIELD2 { start_bit: 8, end_bit: 16, access: rw }

# ❌ Invalid (overlapping)
field FIELD1 { start_bit: 0, end_bit: 8, access: rw }
field FIELD2 { start_bit: 4, end_bit: 12, access: rw }
```

---

## Integration with CMake

```cmake
# Add code generation as build step
add_custom_command(
    OUTPUT ${CMAKE_CURRENT_BINARY_DIR}/device.h
    COMMAND ${BITN_COMPILER} --compile ${CMAKE_CURRENT_SOURCE_DIR}/device.bitn
    MAIN_DEPENDENCY ${CMAKE_CURRENT_SOURCE_DIR}/device.bitn
    WORKING_DIRECTORY ${CMAKE_CURRENT_BINARY_DIR}
)

add_library(device_headers ${CMAKE_CURRENT_BINARY_DIR}/device.h)
```

---

## File Organization

```
project/
├── bitn/
│   ├── uart.bitn
│   ├── gpio.bitn
│   └── timer.bitn
├── include/
│   ├── uart.h          # Generated
│   ├── gpio.h          # Generated
│   └── timer.h         # Generated
└── src/
    ├── main.c
    └── drivers/
```

---

## What Gets Generated

For each peripheral definition, you get:

✅ **Memory-mapped structure** - Correctly sized and aligned
✅ **Base address macro** - Easy peripheral access
✅ **Register fields** - Proper offsets
✅ **Bit field getters** - Safe field reading
✅ **Bit field setters** - Safe field modification
✅ **Inline functions** - Zero-overhead abstractions

---

## Performance

Generated code:
- **Compiles to optimal machine instructions**
- **No runtime overhead**
- **Inlined by C compiler**
- **Zero-cost abstractions**

Example optimization:
```c
// What you write:
uart->CONTROL = CONTROL_BAUDRATE_SET(uart->CONTROL, 9600);

// What assembler sees (approximate):
mov eax, DWORD PTR [rsi]      // Load CONTROL
and eax, 0xfffffe              // Clear baudrate bits
or eax, 0x25b8                 // Set new baudrate
mov DWORD PTR [rsi], eax       // Store back
```

---

## Current Capabilities

### Works ✅
- Parse peripheral definitions
- Generate C headers
- Create register structures
- Generate bit field macros
- Create accessor functions
- Handle multiple devices
- Automatic file naming
- Command-line compilation

### Planned 📅
- Interrupt configuration
- DMA descriptors
- Advanced bitfield patterns
- Code templates
- Documentation generation

---

## Examples

See `EXAMPLE_uart_device.bitn` in workspace for a complete working example.

---

## For More Information

- **Implementation Details:** See `IMPLEMENTATION.md`
- **Development Roadmap:** See `ROADMAP.md`
- **Strategic Vision:** See `VISION.md`

---

**Ready to generate C headers from device definitions!** 🚀
