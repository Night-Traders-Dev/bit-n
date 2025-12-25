// include/core/registers.h
#ifndef BITN_CORE_REGISTERS_H
#define BITN_CORE_REGISTERS_H

#include <stdint.h>
#include <stdbool.h>

/* ARM Cortex-M Register Definitions */

typedef struct {
    uint32_t r[13];        // R0-R12 general purpose
    uint32_t sp;           // R13 (Stack Pointer)
    uint32_t lr;           // R14 (Link Register)
    uint32_t pc;           // R15 (Program Counter)
    uint32_t psr;          // Program Status Register
    
    // Exception masks
    uint32_t primask;      // Primary Interrupt Mask (M0+)
    uint32_t basepri;      // Base Priority (M33)
    uint32_t faultmask;    // Fault Mask (M33)
    uint32_t control;      // Control Register
    
    // M33-specific registers
    uint32_t msplim;       // Main Stack Pointer Limit
    uint32_t psplim;       // Process Stack Pointer Limit
    uint32_t fpscr;        // Floating-Point Status & Control Register
    
    // Execution state
    bool thumb_mode;       // Thumb mode (always true for M0+/M33)
    bool in_exception;     // Currently in exception handler
    uint8_t exception_level; // 0=thread, 1+=handler
} arm_core_state_t;

/* RISC-V Core Register Definitions */

typedef struct {
    uint32_t x[32];        // x0-x31 registers
    uint32_t pc;           // Program counter
    
    // CSRs (Control & Status Registers)
    uint32_t mstatus;      // Machine Status
    uint32_t mie;          // Machine Interrupt Enable
    uint32_t mtvec;        // Machine Trap Vector
    uint32_t mscratch;     // Machine Scratch
    uint32_t mepc;         // Machine Exception PC
    uint32_t mcause;       // Machine Exception Cause
    uint32_t mtval;        // Machine Trap Value
    uint32_t mip;          // Machine Interrupt Pending
    
    uint32_t cycle;        // Cycle counter
    uint32_t instret;      // Instruction retirement counter
    
    // Execution state
    bool compressed_next;  // Next instruction is compressed (16-bit)
    bool in_exception;
} riscv_core_state_t;

/* PSR Flag Bits (ARM) */
#define PSR_N_BIT    (1 << 31)  // Negative flag
#define PSR_Z_BIT    (1 << 30)  // Zero flag
#define PSR_C_BIT    (1 << 29)  // Carry flag
#define PSR_V_BIT    (1 << 28)  // Overflow flag
#define PSR_Q_BIT    (1 << 27)  // Sticky Overflow flag
#define PSR_T_BIT    (1 << 24)  // Thumb mode bit (always 1)
#define PSR_IPSR_MASK 0xFF      // ISR number mask

/* MSTATUS Flag Bits (RISC-V) */
#define MSTATUS_MIE  (1 << 3)   // Machine Interrupt Enable
#define MSTATUS_MPIE (1 << 7)   // Machine Prior Interrupt Enable

/* Public API */
void registers_init_arm(arm_core_state_t *state);
void registers_init_riscv(riscv_core_state_t *state);

uint32_t arm_get_register(arm_core_state_t *state, int reg_num);
void arm_set_register(arm_core_state_t *state, int reg_num, uint32_t value);

uint32_t riscv_get_register(riscv_core_state_t *state, int reg_num);
void riscv_set_register(riscv_core_state_t *state, int reg_num, uint32_t value);

#endif // BITN_CORE_REGISTERS_H
