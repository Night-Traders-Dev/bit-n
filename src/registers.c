// src/core/registers.c
#include "core/registers.h"
#include <string.h>

/**
 * Initialize ARM core registers to default values
 */
void registers_init_arm(arm_core_state_t *state)
{
    if (!state) return;
    
    memset(state, 0, sizeof(arm_core_state_t));
    
    // PSR: Set Thumb mode bit (bit 24) and initial flags
    state->psr = PSR_T_BIT;
    
    // Initialize stack pointer to end of SRAM
    state->sp = 0x10000000 + SRAM_SIZE;
    
    // PC starts at 0 (will be set by bootloader)
    state->pc = 0;
    
    // Thumb mode always enabled for Cortex-M
    state->thumb_mode = true;
    
    // Initially in thread mode
    state->in_exception = false;
    state->exception_level = 0;
}

/**
 * Initialize RISC-V core registers to default values
 */
void registers_init_riscv(riscv_core_state_t *state)
{
    if (!state) return;
    
    memset(state, 0, sizeof(riscv_core_state_t));
    
    // MSTATUS: Machine mode, interrupts disabled
    state->mstatus = 0x1800;  // MPP field = 11 (machine mode)
    
    // Stack pointer (x2) initialized
    state->x[2] = 0x10000000 + SRAM_SIZE;
    
    // PC starts at 0
    state->pc = 0;
    
    // Not in exception
    state->in_exception = false;
}

/**
 * Get ARM register value by register number
 * R0-R12: 0-12
 * SP (R13): 13
 * LR (R14): 14
 * PC (R15): 15
 * PSR: 16
 */
uint32_t arm_get_register(arm_core_state_t *state, int reg_num)
{
    if (!state) return 0;
    
    switch (reg_num) {
        case 0 ... 12:
            return state->r[reg_num];
        case 13:
            return state->sp;
        case 14:
            return state->lr;
        case 15:
            return state->pc;
        case 16:
            return state->psr;
        default:
            return 0;
    }
}

/**
 * Set ARM register value by register number
 */
void arm_set_register(arm_core_state_t *state, int reg_num, uint32_t value)
{
    if (!state) return;
    
    switch (reg_num) {
        case 0 ... 12:
            state->r[reg_num] = value;
            break;
        case 13:
            state->sp = value;
            break;
        case 14:
            state->lr = value;
            break;
        case 15:
            state->pc = value & ~1;  // Clear Thumb bit, handled separately
            break;
        case 16:
            state->psr = (state->psr & 0xFF000000) | (value & 0x00FFFFFF);
            break;
    }
}

/**
 * Get RISC-V register value by register number
 * x0-x31: 0-31
 * PC: 32
 * mstatus: 33
 */
uint32_t riscv_get_register(riscv_core_state_t *state, int reg_num)
{
    if (!state) return 0;
    
    if (reg_num < 32) {
        return state->x[reg_num];
    } else if (reg_num == 32) {
        return state->pc;
    } else if (reg_num == 33) {
        return state->mstatus;
    }
    
    return 0;
}

/**
 * Set RISC-V register value by register number
 */
void riscv_set_register(riscv_core_state_t *state, int reg_num, uint32_t value)
{
    if (!state) return;
    
    if (reg_num < 32) {
        if (reg_num != 0) {  // x0 is hardwired to 0
            state->x[reg_num] = value;
        }
    } else if (reg_num == 32) {
        state->pc = value;
    } else if (reg_num == 33) {
        state->mstatus = value;
    }
}
