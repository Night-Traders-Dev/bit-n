/* rp2350.ld - RP2350 Linker Script for bit(n) Emulator
   
   Memory Layout (Enhanced from RP2040):
   - Flash: 0x10000000 (0-16MB external, or up to 2MB stacked)
   - SRAM:  0x20000000 (520KB total, 10 banks of varying sizes)
   - Boot ROM: 0x00000000 (enhanced, security features)
   
   Notable changes vs RP2040:
   - More SRAM banks (10 vs 6)
   - Additional security/OTP area
   - Support for mixed ARM/RISC-V boot modes
*/

MEMORY
{
    /* Boot ROM (enhanced security, 16KB+) */
    BOOTROM (rx) : ORIGIN = 0x00000000, LENGTH = 32K
    
    /* External Flash via XIP (up to 16MB) */
    FLASH_EXT (rx) : ORIGIN = 0x10000000, LENGTH = 16M
    
    /* Optional Stacked Flash (on-package, 2MB) */
    FLASH_INT (rx) : ORIGIN = 0x12000000, LENGTH = 2M
    
    /* SRAM (520KB at 0x20000000) */
    /* Bank 0-7: 64KB each (512KB) */
    /* Bank 8-9: 4KB each (8KB) */
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 520K
    
    /* SIO (Single-Cycle IO) */
    SIO (rw) : ORIGIN = 0x20081800, LENGTH = 4K
    
    /* Security/OTP area (RP2350 specific) */
    SECURITY (rw) : ORIGIN = 0x20082000, LENGTH = 8K
}

/* Output sections */
SECTIONS
{
    /* Code section in external flash (most common) */
    .text : {
        . = ALIGN(4);
        KEEP(*(.vectors))
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH_EXT
    
    /* Initialized data section (loaded in flash, copied to SRAM) */
    .data : {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH_EXT
    
    /* Uninitialized data section (BSS) */
    .bss : {
        . = ALIGN(4);
        _sbss = .;
        *(.bss)
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _ebss = .;
    } > SRAM
    
    /* Security-critical data (optional, in security area) */
    .security : {
        . = ALIGN(4);
        *(.security)
        *(.security*)
    } > SECURITY
    
    /* Stack at end of SRAM */
    .stack : {
        . = ALIGN(4);
        _stack_end = .;
        . += 8K;
        _stack_start = .;
    } > SRAM
    
    /* Heap follows stack */
    .heap : {
        . = ALIGN(4);
        _heap_start = .;
        . = . + 16K;
        _heap_end = .;
    } > SRAM
}

/* Symbol definitions */
_sidata = LOADADDR(.data);
_eidata = _sidata + SIZEOF(.data);
_security_base = ORIGIN(SECURITY);
_security_size = LENGTH(SECURITY);

/* For ARM TrustZone (RP2350 feature) */
_secure_start = ORIGIN(SRAM);
_secure_end = ORIGIN(SRAM) + (512K / 2);  /* First half secure */
_non_secure_start = _secure_end;
_non_secure_end = ORIGIN(SRAM) + 512K;
