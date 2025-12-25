// include/isa/arm/decoder.h
#ifndef BITN_ARM_DECODER_H
#define BITN_ARM_DECODER_H

#include <stdint.h>
#include <stdbool.h>
#include "core/registers.h"

/* Thumb-2 Instruction Decoder */

/* Opcode constants */
#define THUMB_UNCONDITIONAL_MASK 0xF800
#define THUMB_UNCONDITIONAL      0xE000

/* Instruction types */
typedef enum {
    ARM_INSTR_INVALID = 0,
    ARM_INSTR_ADD,
    ARM_INSTR_SUB,
    ARM_INSTR_MOV,
    ARM_INSTR_LDR,
    ARM_INSTR_STR,
    ARM_INSTR_B,
    ARM_INSTR_BL,
    ARM_INSTR_AND,
    ARM_INSTR_ORR,
    ARM_INSTR_XOR,
    ARM_INSTR_LDM,
    ARM_INSTR_STM,
    ARM_INSTR_CMP,
    ARM_INSTR_TST,
    ARM_INSTR_BX,
    ARM_INSTR_BLX,
    ARM_INSTR_MRS,
    ARM_INSTR_MSR,
    ARM_INSTR_PUSH,
    ARM_INSTR_POP,
    ARM_INSTR_NOP,
    ARM_INSTR_WFI,
    ARM_INSTR_SVC,
} arm_instruction_type_t;

typedef struct {
    arm_instruction_type_t type;
    uint32_t raw_instruction;
    uint8_t condition;
    
    /* Operands */
    uint8_t rd, rs, rm, rn;
    int32_t immediate;
    bool setflags;
    uint32_t shift_amount;
    uint8_t shift_type;  /* 0=LSL, 1=LSR, 2=ASR, 3=ROR */
    
} arm_instruction_t;

/* Public API */
int arm_thumb2_decode(uint32_t instruction, uint8_t instr_len, arm_instruction_t *instr);
int arm_thumb2_execute(arm_core_state_t *core, uint32_t instruction, uint8_t instr_len);

/* Helper functions */
uint32_t arm_decode_imm12(uint32_t imm12);
uint32_t arm_decode_imm8_rotated(uint8_t imm8, uint8_t rotate);
bool arm_check_condition(arm_core_state_t *core, uint8_t condition);

#endif // BITN_ARM_DECODER_H
