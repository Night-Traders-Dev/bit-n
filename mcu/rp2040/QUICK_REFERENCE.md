# Quick Reference - RP2040 Peripherals

Fast lookup for common operations with each peripheral.

---

## GPIO

**File:** `gpio.bitn`  
**Base:** 0x40014000  
**Registers:** 4

| Register | Offset | Purpose |
|----------|--------|---------|
| GPIO_OE | 0x20 | Output Enable (write to enable pin as output) |
| GPIO_IN | 0x24 | Input Values (read pin states) |
| GPIO_OUT | 0x28 | Output Values (write pin values) |
| GPIO_OD | 0x2C | Open-Drain Control |

**Example - Set GPIO 25 (LED) as output:**
```bitn
peripheral GPIO @ 0x40014000 {
    register GPIO_OE: u32 @ 0x20 {
        field OE_25: [25:25] rw;
    }
}

fn setup_led() -> u32 {
    return shl(1, 25);  // Set bit 25
}
```

---

## UART

**Files:** `uart.bitn`  
**Base:** UART0 @ 0x40034000, UART1 @ 0x40038000  
**Registers:** 11 per UART

| Register | Offset | Purpose |
|----------|--------|---------|
| UARTDR | 0x00 | Data register (TX/RX) |
| UARTFR | 0x18 | Flags (TXFE, RXFE, BUSY, etc.) |
| UARTIBRD | 0x24 | Integer baud rate divisor |
| UARTFBRD | 0x28 | Fractional baud rate divisor |
| UARTLCR_H | 0x2C | Line control (word length, stop bits) |
| UARTCR | 0x30 | Control (UARTEN, TXE, RXE) |

**Quick Setup:**
1. Set UARTIBRD and UARTFBRD for baud rate
2. Set UARTLCR_H for word length/stop bits
3. Set UARTCR to enable (UARTEN=1, TXE=1, RXE=1)

---

## SPI

**File:** `spi.bitn`  
**Base:** SPI0 @ 0x4003C000, SPI1 @ 0x40040000  
**Registers:** 9 per SPI

| Register | Offset | Purpose |
|----------|--------|---------|
| SSPCR0 | 0x00 | Control (clock phase/polarity, data size) |
| SSPCR1 | 0x04 | Control (enable, master mode) |
| SSPDR | 0x08 | Data FIFO |
| SSPSR | 0x0C | Status (TFE, TNF, RNE, RFF) |
| SSPCPSR | 0x10 | Clock prescale divisor |

**Quick Setup:**
1. Set SSPCPSR for clock divider
2. Set SSPCR0 for data size and clock settings
3. Set SSPCR1 to enable (SSE=1, MS=master)
4. Write data to SSPDR, read from SSPDR

---

## I2C

**File:** `i2c.bitn`  
**Base:** I2C0 @ 0x40044000, I2C1 @ 0x40048000  
**Registers:** 17 per I2C

| Register | Offset | Purpose |
|----------|--------|---------|
| IC_CON | 0x00 | Configuration (speed, addressing) |
| IC_TAR | 0x04 | Target slave address |
| IC_DATA_CMD | 0x10 | Data and command (read/write) |
| IC_SS_SCL_HCNT | 0x14 | Standard speed SCL high time |
| IC_SS_SCL_LCNT | 0x18 | Standard speed SCL low time |
| IC_ENABLE | 0x6C | Enable the controller |
| IC_STATUS | 0x70 | Status (busy, FIFO levels) |

**Quick Setup:**
1. Set IC_CON for speed mode and addressing
2. Set IC_SS_SCL_HCNT and IC_SS_SCL_LCNT for timing
3. Set IC_TAR for target slave address
4. Set IC_ENABLE to 1
5. Write/read via IC_DATA_CMD

---

## Common Field Patterns

### Access Modes
- `ro` - Read Only (hardware sets, software reads)
- `wo` - Write Only (software writes, hardware reads)
- `rw` - Read/Write (bidirectional)
- `w1c` - Write 1 to Clear (common for status/interrupt bits)

### Bit Ranges
- `[0:0]` - Single bit (bit 0)
- `[7:0]` - Byte (bits 0-7, 8 bits)
- `[15:8]` - Upper byte (bits 8-15)
- `[31:16]` - Upper half-word (bits 16-31)

### Common Interrupt Patterns
```bitn
register INT_MASK: u32 @ 0x10 {
    field TX_INT: [0:0]  rw;   // TX interrupt enable
    field RX_INT: [1:1]  rw;   // RX interrupt enable
}

register INT_STATUS: u32 @ 0x14 {
    field TX_PENDING: [0:0]  ro;   // TX interrupt pending
    field RX_PENDING: [1:1]  ro;   // RX interrupt pending
}

register INT_CLEAR: u32 @ 0x18 {
    field TX_CLEAR: [0:0]  wo;   // Write 1 to clear TX
    field RX_CLEAR: [1:1]  wo;   // Write 1 to clear RX
}
```

---

## Memory Map Quick Reference

```
GPIO:     0x40014000 - 0x4001403F (64 bytes)
UART0:    0x40034000 - 0x400343FF (1 KB)
UART1:    0x40038000 - 0x400383FF (1 KB)
SPI0:     0x4003C000 - 0x4003CFFF (4 KB)
SPI1:     0x40040000 - 0x40040FFF (4 KB)
I2C0:     0x40044000 - 0x40044FFF (4 KB)
I2C1:     0x40048000 - 0x40048FFF (4 KB)
```

---

## Testing Commands

```bash
# Test all peripherals compile
./build/bitN mcu/rp2040/gpio.bitn
./build/bitN mcu/rp2040/uart.bitn
./build/bitN mcu/rp2040/spi.bitn
./build/bitN mcu/rp2040/i2c.bitn

# Expected output for each
# ✅ Successfully parsed
#    Peripherals: 1-2
```

---

## Code Examples

### Enable GPIO Output
```bitn
peripheral GPIO @ 0x40014000 {
    register GPIO_OE: u32 @ 0x20 {
        field OE: [25:25] rw;
    }
}

fn enable_output() -> u32 = 1  // Enable output
```

### UART Transmit
```bitn
peripheral UART @ 0x40034000 {
    register DR: u32 @ 0x00 {
        field DATA: [7:0] rw;
    }
}

fn send_byte(c: u32) -> u32 = c
```

### SPI Transfer
```bitn
peripheral SPI @ 0x4003C000 {
    register DR: u32 @ 0x08 {
        field DATA: [15:0] rw;
    }
}

fn transfer(data: u32) -> u32 = data
```

### I2C Write
```bitn
peripheral I2C @ 0x40044000 {
    register DATA_CMD: u32 @ 0x10 {
        field DAT: [7:0] rw;
        field CMD: [8:8] rw;
    }
}

fn write_byte(data: u32) -> u32 = data
```

---

## Debug Tips

1. **Check base addresses** - Verify against datasheet
2. **Verify register offsets** - Common mistakes: 0x00 vs 0x20
3. **Confirm field ranges** - e.g., GPIO fields should be [0:25]
4. **Check access modes** - ro vs rw vs wo vs w1c
5. **Test compilation** - Each file should parse with zero errors

---

**Last Updated:** December 15, 2025  
**Version:** Phase 2 Complete