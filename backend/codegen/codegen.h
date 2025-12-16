/**
 * bit(N) Compiler - C Code Generator Header
 * Corrected for your actual AST structure
 */

#ifndef BITN_CODEGEN_H
#define BITN_CODEGEN_H

#include "../../include/ast.h"
#include <stdio.h>

typedef struct {
    FILE *output;
    int indent_level;
    char *target_arch;
    char *target_abi;
    int use_volatile;
    int inline_asm;
} CodegenContext;

CodegenContext* codegen_init(const char *output_file, const char *target);
int codegen_generate(CodegenContext *ctx, ASTProgram *program);
int codegen_peripheral(CodegenContext *ctx, ASTPeripheral *periph);
int codegen_register(CodegenContext *ctx, ASTRegister *reg);
int codegen_field_accessors(CodegenContext *ctx, ASTRegister *reg);
int codegen_helpers(CodegenContext *ctx);
void codegen_cleanup(CodegenContext *ctx);
void codegen_indent(CodegenContext *ctx);
void codegen_write(CodegenContext *ctx, const char *fmt, ...);
char* sanitize_identifier(const char *name);

#endif // BITN_CODEGEN_H
