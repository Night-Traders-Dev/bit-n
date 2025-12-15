# Raspberry Pi Pico (RP2040) MCU Definitions

> Comprehensive peripheral definitions for the RP2040 microcontroller in bit(N) DSL

---

## Overview

The RP2040 is a dual-core Arm Cortex-M0+ microcontroller with:
- **Flash:** 2MB
- **RAM:** 264KB
- **Clock:** Up to 133MHz
- **GPIO Pins:** 26 (plus 3 QSPI pins)
- **Peripherals:** GPIO, UART, SPI, I2C, PWM, ADC, timers, DMA, PIO

This directory contains bit(N) DSL definitions for RP2040 peripherals.

---

## File Structure

```
mcu/rp2040/
├── README.md                 # This file
├── gpio.bitn                 # GPIO controller
├── uart.bitn                 # UART serial interface
├── spi.bitn                  # SPI interface
├── i2c.bitn                  # I2C interface
├── pwm.bitn                  # PWM controller
├── adc.bitn                  # ADC converter
├── timer.bitn                # System timer
├── dma.bitn                  # DMA controller
└── pio.bitn                  # PIO state machines
```

---

## Memory Map

```
Peripheral          Address Range      Size
────────────────────────────────────────────────
GPIO                0x40014000-0x4001403F   64B
UART0               0x40034000-0x400343FF  1KB
UART1               0x40038000-0x400383FF  1KB
SPI0                0x4003C000-0x4003CFFF  4KB
SPI1                0x40040000-0x40040FFF  4KB
I2C0                0x40044000-0x40044FFF  4KB
I2C1                0x40048000-0x40048FFF  4KB
PWM                 0x40050000-0x40050FFF  4KB
ADC                 0x4004C000-0x4004CFFF  4KB
Timer               0x40054000-0x40054FFF  4KB
DMA                 0x50000000-0x5000EFFF  60KB
PIO0                0x50200000-0x50200FFF  4KB
PIO1                0x50300000-0x50300FFF  4KB
```

---

## Status

| Peripheral | Status | Lines | Notes |
|-----------|--------|-------|-------|
| GPIO      | ✅ Done | ~80   | Full GPIO_OE, GPIO_IN, GPIO_OUT, GPIO_OD |
| UART      | 📅 TODO | -     | Planned |
| SPI       | 📅 TODO | -     | Planned |
| I2C       | 📅 TODO | -     | Planned |
| PWM       | 📅 TODO | -     | Planned |
| ADC       | 📅 TODO | -     | Planned |
| Timer     | 📅 TODO | -     | Planned |
| DMA       | 📅 TODO | -     | Planned |
| PIO       | 📅 TODO | -     | Planned |

---

## Completed Definitions

### GPIO (✅ Complete)

**File:** `gpio.bitn`

Defines GPIO peripheral at `0x40014000` with:
- **GPIO_OE** (0x20) - Output enable register
  - Fields: OE_0 through OE_3 [rw]
- **GPIO_IN** (0x24) - Input value register
  - Fields: IN_0 through IN_3 [ro]
- **GPIO_OUT** (0x28) - Output value register
  - Fields: OUT_0 through OUT_3 [rw]

**Usage:**
```bitn
// Include in your project
peripheral GPIO @ 0x40014000 {
    register GPIO_OE: u32 @ 0x20 {
        field OE_0: [0:1]   rw;
        field OE_1: [1:2]   rw;
        field OE_2: [2:3]   rw;
        field OE_3: [3:4]   rw;
    }
    
    register GPIO_IN: u32 @ 0x24 {
        field IN_0: [0:1]   ro;
        field IN_1: [1:2]   ro;
        field IN_2: [2:3]   ro;
        field IN_3: [3:4]   ro;
    }
    
    register GPIO_OUT: u32 @ 0x28 {
        field OUT_0: [0:1]   rw;
        field OUT_1: [1:2]   rw;
        field OUT_2: [2:3]   rw;
        field OUT_3: [3:4]   rw;
    }
}
```

---

## Upcoming Definitions

### UART (📅 Planned)

Will define UART0 and UART1 with:
- **UARTDR** - Data register
- **UARTFR** - Flag register (TXFE, RXFE, TXFF, RXFF)
- **UARTIBRD** - Integer baud rate divisor
- **UARTFBRD** - Fractional baud rate divisor
- **UARTLCR_H** - Line control register (WLEN, STP2, EPS, PEN)
- **UARTCR** - Control register (UART enable, TX enable, RX enable)

### SPI (📅 Planned)

Will define SPI0 and SPI1 with:
- **SSPCR0** - Control register 0 (SPO, SPH, FRF, DSS)
- **SSPCR1** - Control register 1 (LBM, SSE, MS, SOD)
- **SSPDR** - Data register
- **SSPSR** - Status register (TFE, TNF, RNE, RFF, BSY)

### I2C (📅 Planned)

Will define I2C0 and I2C1 with:
- **IC_CON** - Configuration register
- **IC_TAR** - Target address register
- **IC_DATA_CMD** - Data and command register
- **IC_STATUS** - Status register
- **IC_FS_SCL_HCNT/LCNT** - Fast speed timing

### PWM (📅 Planned)

Will define PWM channels with:
- **CH0_CSR** - Channel control/status
- **CH0_CTR** - Counter
- **CH0_CC** - Comparator count
- **CH0_TOP** - Counter wrap

### ADC (📅 Planned)

Will define ADC with:
- **FIFO** - ADC conversion result FIFO
- **FSTAT** - FIFO status register
- **FCS** - FIFO control and status
- **INTR** - Interrupt status register

### Timer (📅 Planned)

Will define system timer with:
- **TIMELR** - Lower 32 bits of timer counter
- **TIMEHR** - Upper 32 bits of timer counter
- **TIMEDIFF0-3** - Alarm comparators
- **INTS** - Interrupt status register

### DMA (📅 Planned)

Will define DMA channels and control with:
- **CHx_READ_ADDR** - Read address pointer
- **CHx_WRITE_ADDR** - Write address pointer
- **CHx_TRANS_COUNT** - Transfer count
- **CHx_CTRL_TRIG** - Control register with trigger

### PIO (📅 Planned)

Will define PIO state machines with:
- **CTRL** - Control register
- **FSTAT** - FIFO status
- **FDEBUG** - FIFO debug
- **SMx_CLKDIV** - Clock divider
- **SMx_EXECCTRL** - Execution control
- **SMx_SHIFTCTRL** - Shift control
- **SMx_ADDR** - Program address
- **SMx_INSTR** - Instruction

---

## Contributing

To add a new peripheral:

1. Create `peripheral_name.bitn` in this directory
2. Use the GPIO template as reference
3. Update this README with status
4. Include full register map with fields
5. Document field widths and access modes

### Field Access Modes

- **ro** - Read-only (hardware updates)
- **wo** - Write-only (write triggers action)
- **rw** - Read-write (general purpose)
- **w1c** - Write-1-to-clear (interrupt flags)

### Bit Range Syntax

```bitn
field NAME: [START:END] MODE;
```

Where:
- `START` = bit index (inclusive)
- `END` = bit index (inclusive)
- Range `[0:1]` = bits 0 and 1

---

## References

- **RP2040 Datasheet:** https://datasheets.raspberrypi.com/rp2040/rp2040-datasheet.pdf
- **Pico SDK:** https://github.com/raspberrypi/pico-sdk
- **Pico Documentation:** https://www.raspberrypi.com/documentation/microcontrollers/rp2040.html

---

## Template for New Peripherals

```bitn
// PERIPHERAL_NAME Peripheral Definition
// Base: 0xYYYYYYYY
// Description: What this peripheral does

peripheral PERIPHERAL_NAME @ 0xYYYYYYYY {
    // Register 1
    register REG_NAME1: u32 @ 0x00 {
        field FIELD1: [0:0]   rw;   // Bit 0
        field FIELD2: [1:1]   rw;   // Bit 1
        field FIELD3: [7:2]   rw;   // Bits 2-7
    }
    
    // Register 2
    register REG_NAME2: u32 @ 0x04 {
        field FIELD4: [15:0]  ro;   // Bits 0-15
        field FIELD5: [31:16] rw;   // Bits 16-31
    }
}
```

---

**Status:** Phase 2 Complete - GPIO definitions ready, other peripherals planned for Phase 3

---