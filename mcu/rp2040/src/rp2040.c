// src/rp2040/rp2040.c
#include "rp2040/rp2040.h"
#include "isa/arm/decoder.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/**
 * Create and initialize an RP2040 system
 */
rp2040_system_t *rp2040_create(void)
{
    rp2040_system_t *sys = (rp2040_system_t *)malloc(sizeof(rp2040_system_t));
    if (!sys) {
        fprintf(stderr, "Failed to allocate RP2040 system\n");
        return NULL;
    }
    
    memset(sys, 0, sizeof(rp2040_system_t));
    
    /* Allocate cores */
    for (int i = 0; i < RP2040_NUM_CORES; i++) {
        sys->cores[i] = (arm_core_state_t *)malloc(sizeof(arm_core_state_t));
        if (!sys->cores[i]) {
            fprintf(stderr, "Failed to allocate core %d\n", i);
            rp2040_destroy(sys);
            return NULL;
        }
        registers_init_arm(sys->cores[i]);
    }
    
    /* Allocate SRAM */
    sys->sram = (sram_t *)malloc(sizeof(sram_t));
    if (!sys->sram) {
        fprintf(stderr, "Failed to allocate SRAM\n");
        rp2040_destroy(sys);
        return NULL;
    }
    sram_init(sys->sram, RP2040_SRAM_SIZE);
    
    /* Allocate GPIO */
    sys->gpio = (gpio_state_t *)malloc(sizeof(gpio_state_t));
    if (!sys->gpio) {
        fprintf(stderr, "Failed to allocate GPIO\n");
        rp2040_destroy(sys);
        return NULL;
    }
    gpio_init(sys->gpio, RP2040_GPIO_PINS);
    
    /* Allocate UARTs */
    for (int i = 0; i < 2; i++) {
        sys->uart[i] = (uart_state_t *)malloc(sizeof(uart_state_t));
        if (!sys->uart[i]) {
            fprintf(stderr, "Failed to allocate UART %d\n", i);
            rp2040_destroy(sys);
            return NULL;
        }
        uart_init(sys->uart[i]);
    }
    
    /* Allocate AHB bus */
    sys->ahb_bus = (ahb_interconnect_t *)malloc(sizeof(ahb_interconnect_t));
    if (!sys->ahb_bus) {
        fprintf(stderr, "Failed to allocate AHB bus\n");
        rp2040_destroy(sys);
        return NULL;
    }
    ahb_interconnect_init(sys->ahb_bus);
    
    /* Initialize system state */
    sys->cycle_count = 0;
    sys->clock_freq = RP2040_CLOCK_HZ;
    sys->halted = false;
    sys->breakpoint_triggered = false;
    sys->num_breakpoints = 0;
    sys->active_core = 0;
    
    return sys;
}

/**
 * Destroy and free RP2040 system
 */
void rp2040_destroy(rp2040_system_t *sys)
{
    if (!sys) return;
    
    for (int i = 0; i < RP2040_NUM_CORES; i++) {
        if (sys->cores[i]) free(sys->cores[i]);
    }
    
    if (sys->sram) {
        sram_destroy(sys->sram);
        free(sys->sram);
    }
    
    if (sys->gpio) {
        gpio_destroy(sys->gpio);
        free(sys->gpio);
    }
    
    for (int i = 0; i < 2; i++) {
        if (sys->uart[i]) {
            uart_destroy(sys->uart[i]);
            free(sys->uart[i]);
        }
    }
    
    if (sys->ahb_bus) {
        ahb_interconnect_destroy(sys->ahb_bus);
        free(sys->ahb_bus);
    }
    
    free(sys);
}

/**
 * Load an ELF file into emulator memory
 */
int rp2040_load_elf(rp2040_system_t *sys, const char *filename)
{
    if (!sys || !filename) return -1;
    
    FILE *f = fopen(filename, "rb");
    if (!f) {
        fprintf(stderr, "Failed to open file: %s\n", filename);
        return -1;
    }
    
    // TODO: Parse ELF header, load segments into memory
    // For now, simplified: load file directly at 0x20000000
    
    uint8_t buffer[4096];
    uint32_t addr = RP2040_SRAM_BASE;
    
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, sizeof(buffer), f)) > 0) {
        if (rp2040_load_binary(sys, addr, buffer, bytes_read) < 0) {
            fprintf(stderr, "Failed to load binary data\n");
            fclose(f);
            return -1;
        }
        addr += bytes_read;
    }
    
    fclose(f);
    
    /* Set PC to SRAM start */
    sys->cores[0]->pc = RP2040_SRAM_BASE;
    
    return 0;
}

/**
 * Load binary data at specific address
 */
int rp2040_load_binary(rp2040_system_t *sys, uint32_t addr, const uint8_t *data, uint32_t len)
{
    if (!sys || !data) return -1;
    
    /* Validate address range */
    if (addr < RP2040_SRAM_BASE || addr + len > RP2040_SRAM_BASE + RP2040_SRAM_SIZE) {
        fprintf(stderr, "Load address out of range: 0x%08x\n", addr);
        return -1;
    }
    
    uint32_t offset = addr - RP2040_SRAM_BASE;
    memcpy(sys->sram->data + offset, data, len);
    
    return 0;
}

/**
 * Step a single core through one instruction
 */
int rp2040_step_core(rp2040_system_t *sys, int core_id)
{
    if (!sys || core_id < 0 || core_id >= RP2040_NUM_CORES) {
        return -1;
    }
    
    arm_core_state_t *core = sys->cores[core_id];
    
    /* Check breakpoint */
    for (int i = 0; i < sys->num_breakpoints; i++) {
        if (sys->breakpoints[i] == core->pc) {
            sys->breakpoint_triggered = true;
            return 1;  /* Breakpoint hit */
        }
    }
    
    /* Fetch instruction from memory */
    uint32_t pc = core->pc;
    uint16_t hw1, hw2;
    
    hw1 = sram_read_halfword(sys->sram, pc - RP2040_SRAM_BASE);
    
    /* Determine if 16-bit or 32-bit instruction */
    uint32_t instr;
    uint8_t instr_len = 2;  /* bytes */
    
    if ((hw1 & 0xE000) == 0xE000 && (hw1 & 0x1800) != 0) {
        /* 32-bit Thumb-2 instruction */
        hw2 = sram_read_halfword(sys->sram, pc + 2 - RP2040_SRAM_BASE);
        instr = (hw2 << 16) | hw1;
        instr_len = 4;
    } else {
        /* 16-bit Thumb instruction */
        instr = hw1;
        instr_len = 2;
    }
    
    /* Decode and execute */
    int result = arm_thumb2_execute(core, instr, instr_len);
    if (result < 0) {
        fprintf(stderr, "Execution error at 0x%08x: 0x%08x\n", pc, instr);
        return -1;
    }
    
    /* Increment cycle count */
    sys->cycle_count++;
    
    return 0;
}

/**
 * Step both cores (round-robin)
 */
int rp2040_step(rp2040_system_t *sys)
{
    if (!sys) return -1;
    
    static int last_core = 0;
    int result = rp2040_step_core(sys, last_core);
    
    last_core = (last_core + 1) % RP2040_NUM_CORES;
    
    return result;
}

/**
 * Run until halted or breakpoint
 */
int rp2040_run_until_halt(rp2040_system_t *sys)
{
    if (!sys) return -1;
    
    while (!sys->halted && !sys->breakpoint_triggered) {
        if (rp2040_step(sys) < 0) {
            return -1;
        }
    }
    
    return 0;
}

/**
 * Run for specific number of cycles
 */
int rp2040_run_cycles(rp2040_system_t *sys, uint64_t cycles)
{
    if (!sys) return -1;
    
    uint64_t target = sys->cycle_count + cycles;
    
    while (sys->cycle_count < target && !sys->halted && !sys->breakpoint_triggered) {
        if (rp2040_step(sys) < 0) {
            return -1;
        }
    }
    
    return 0;
}

/**
 * Get register value
 */
uint32_t rp2040_get_register(rp2040_system_t *sys, int core_id, int reg_num)
{
    if (!sys || core_id < 0 || core_id >= RP2040_NUM_CORES) {
        return 0;
    }
    
    return arm_get_register(sys->cores[core_id], reg_num);
}

/**
 * Set register value
 */
void rp2040_set_register(rp2040_system_t *sys, int core_id, int reg_num, uint32_t value)
{
    if (!sys || core_id < 0 || core_id >= RP2040_NUM_CORES) {
        return;
    }
    
    arm_set_register(sys->cores[core_id], reg_num, value);
}

/**
 * Read from memory
 */
uint32_t rp2040_read_memory(rp2040_system_t *sys, uint32_t addr)
{
    if (!sys) return 0;
    
    if (addr >= RP2040_SRAM_BASE && addr < RP2040_SRAM_BASE + RP2040_SRAM_SIZE) {
        uint32_t offset = addr - RP2040_SRAM_BASE;
        return sram_read_word(sys->sram, offset);
    }
    
    return 0;
}

/**
 * Write to memory
 */
void rp2040_write_memory(rp2040_system_t *sys, uint32_t addr, uint32_t value)
{
    if (!sys) return;
    
    if (addr >= RP2040_SRAM_BASE && addr < RP2040_SRAM_BASE + RP2040_SRAM_SIZE) {
        uint32_t offset = addr - RP2040_SRAM_BASE;
        sram_write_word(sys->sram, offset, value);
    }
}

/**
 * Add a breakpoint
 */
void rp2040_add_breakpoint(rp2040_system_t *sys, uint32_t addr)
{
    if (!sys || sys->num_breakpoints >= 32) return;
    
    sys->breakpoints[sys->num_breakpoints++] = addr;
}

/**
 * Remove a breakpoint
 */
void rp2040_remove_breakpoint(rp2040_system_t *sys, uint32_t addr)
{
    if (!sys) return;
    
    for (int i = 0; i < sys->num_breakpoints; i++) {
        if (sys->breakpoints[i] == addr) {
            sys->breakpoints[i] = sys->breakpoints[--sys->num_breakpoints];
            break;
        }
    }
}

/**
 * Clear all breakpoints
 */
void rp2040_clear_breakpoints(rp2040_system_t *sys)
{
    if (!sys) return;
    
    sys->num_breakpoints = 0;
}

/**
 * Set GPIO pin value
 */
int rp2040_gpio_set(rp2040_system_t *sys, int pin, bool value)
{
    if (!sys || !sys->gpio) return -1;
    
    return gpio_write_pin(sys->gpio, pin, value);
}

/**
 * Get GPIO pin value
 */
bool rp2040_gpio_get(rp2040_system_t *sys, int pin)
{
    if (!sys || !sys->gpio) return false;
    
    return gpio_read_pin(sys->gpio, pin);
}

/**
 * Write to UART
 */
int rp2040_uart_write(rp2040_system_t *sys, int uart_id, const uint8_t *data, uint32_t len)
{
    if (!sys || uart_id < 0 || uart_id >= 2 || !sys->uart[uart_id]) {
        return -1;
    }
    
    return uart_write(sys->uart[uart_id], data, len);
}

/**
 * Read from UART
 */
int rp2040_uart_read(rp2040_system_t *sys, int uart_id, uint8_t *data, uint32_t len)
{
    if (!sys || uart_id < 0 || uart_id >= 2 || !sys->uart[uart_id]) {
        return -1;
    }
    
    return uart_read(sys->uart[uart_id], data, len);
}
