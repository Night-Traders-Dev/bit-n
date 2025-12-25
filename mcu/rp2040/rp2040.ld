/* rp2040.ld - RP2040 Linker Script for bit(n) Emulator
   
   Memory Layout:
   - Flash: 0x10000000 (0-16MB, external)
   - SRAM:  0x10000000 (264KB total, 6 banks)
   - Boot ROM: 0x00000000 (16KB)
   
   This linker script is used when emulating RP2040 target code,
   not for the emulator itself.
*/

MEMORY
{
    /* Boot ROM (16KB at start of address space) */
    BOOTROM (rx) : ORIGIN = 0x00000000, LENGTH = 16K
    
    /* XIP Flash Virtual (execute-in-place cached) */
    FLASH (rx) : ORIGIN = 0x10000000, LENGTH = 16M
    
    /* SRAM (264KB at 0x20000000) */
    /* Bank 0: 0x20000000 - 64KB */
    /* Bank 1: 0x20010000 - 64KB */
    /* Bank 2: 0x20020000 - 64KB */
    /* Bank 3: 0x20030000 - 64KB */
    /* Bank 4: 0x20040000 - 4KB */
    /* Bank 5: 0x20041000 - 4KB */
    SRAM (rwx) : ORIGIN = 0x20000000, LENGTH = 264K
    
    /* SIO (Single-Cycle IO) - fast GPIO access */
    SIO (rw) : ORIGIN = 0x20041800, LENGTH = 4K
}

/* Output sections */
SECTIONS
{
    /* Code section in flash */
    .text : {
        . = ALIGN(4);
        *(.text)
        *(.text*)
        *(.rodata)
        *(.rodata*)
        . = ALIGN(4);
        _etext = .;
    } > FLASH
    
    /* Initialized data section (loaded in flash, copied to SRAM) */
    .data : {
        . = ALIGN(4);
        _sdata = .;
        *(.data)
        *(.data*)
        . = ALIGN(4);
        _edata = .;
    } > SRAM AT > FLASH
    
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
    
    /* Stack at end of SRAM */
    .stack : {
        . = ALIGN(4);
        _stack_end = .;
        . += 4K;
        _stack_start = .;
    } > SRAM
}

/* Symbol definitions for startup code */
_sidata = LOADADDR(.data);
_eidata = _sidata + SIZEOF(.data);
