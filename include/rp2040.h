// include/rp2040/rp2040.h
#ifndef BITN_RP2040_H
#define BITN_RP2040_H

#include <stdint.h>
#include <stdbool.h>
#include "core/registers.h"
#include "periph/gpio.h"
#include "periph/uart.h"
#include "memory/sram.h"
#include "bus/ahb_lite.h"

/* RP2040 System Configuration */
#define RP2040_SRAM_SIZE        0x42800     /* 264KB */
#define RP2040_SRAM_BASE        0x20000000
#define RP2040_GPIO_PINS        30
#define RP2040_NUM_CORES        2
#define RP2040_CLOCK_HZ         133000000   /* 133 MHz */

/* RP2040 Memory Map */
#define RP2040_BOOTROM_BASE     0x00000000  /* 16KB */
#define RP2040_SRAM_BANK0       0x20000000  /* 64KB */
#define RP2040_SRAM_BANK1       0x20010000  /* 64KB */
#define RP2040_SRAM_BANK2       0x20020000  /* 64KB */
#define RP2040_SRAM_BANK3       0x20030000  /* 64KB */
#define RP2040_SRAM_BANK4       0x20040000  /* 4KB */
#define RP2040_SRAM_BANK5       0x20041000  /* 4KB */

#define RP2040_APB0_BASE        0x40000000
#define RP2040_APB1_BASE        0x50000000
#define RP2040_AHB_BASE         0x50400000
#define RP2040_SIO_BASE         0xd0000000
#define RP2040_XIP_BASE         0x10000000  /* External flash XIP */

/* RP2040 Core Structure */
typedef struct {
    arm_core_state_t *cores[RP2040_NUM_CORES];
    gpio_state_t *gpio;
    uart_state_t *uart[2];
    ahb_interconnect_t *ahb_bus;
    sram_t *sram;
    
    uint64_t cycle_count;
    uint32_t clock_freq;
    bool halted;
    bool breakpoint_triggered;
    
    /* Debugging support */
    uint32_t breakpoints[32];
    uint8_t num_breakpoints;
    uint8_t active_core;  /* Which core to debug */
} rp2040_system_t;

/* Public API */
rp2040_system_t *rp2040_create(void);
void rp2040_destroy(rp2040_system_t *sys);

int rp2040_load_elf(rp2040_system_t *sys, const char *filename);
int rp2040_load_binary(rp2040_system_t *sys, uint32_t addr, const uint8_t *data, uint32_t len);

int rp2040_step(rp2040_system_t *sys);
int rp2040_step_core(rp2040_system_t *sys, int core_id);
int rp2040_run_until_halt(rp2040_system_t *sys);
int rp2040_run_cycles(rp2040_system_t *sys, uint64_t cycles);

/* Debugging */
uint32_t rp2040_get_register(rp2040_system_t *sys, int core_id, int reg_num);
void rp2040_set_register(rp2040_system_t *sys, int core_id, int reg_num, uint32_t value);

uint32_t rp2040_read_memory(rp2040_system_t *sys, uint32_t addr);
void rp2040_write_memory(rp2040_system_t *sys, uint32_t addr, uint32_t value);

void rp2040_add_breakpoint(rp2040_system_t *sys, uint32_t addr);
void rp2040_remove_breakpoint(rp2040_system_t *sys, uint32_t addr);
void rp2040_clear_breakpoints(rp2040_system_t *sys);

/* GPIO/Peripheral Control */
int rp2040_gpio_set(rp2040_system_t *sys, int pin, bool value);
bool rp2040_gpio_get(rp2040_system_t *sys, int pin);

int rp2040_uart_write(rp2040_system_t *sys, int uart_id, const uint8_t *data, uint32_t len);
int rp2040_uart_read(rp2040_system_t *sys, int uart_id, uint8_t *data, uint32_t len);

#endif // BITN_RP2040_H
