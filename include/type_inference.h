#ifndef TYPE_INFERENCE_H
#define TYPE_INFERENCE_H

#include "ast.h"
#include "symbol_table.h"

// Type context tracks symbol table and compilation state
typedef struct {
    SymbolTable *symbols;
    int error_count;
    const char *current_function;  // For return type checking
    Type *expected_return_type;    // Return type of current function
} TypeContext;

// API - Lifecycle
TypeContext *type_context_create(void);
void type_context_free(TypeContext *ctx);

// API - Context setup
void type_context_set_function(TypeContext *ctx, const char *name, Type *return_type);

// API - Type inference
Type *infer_expr_type(TypeContext *ctx, ASTExpr *expr);

// API - Statement validation
int check_stmt_types(TypeContext *ctx, ASTStmt *stmt);

// API - Function validation
int check_function_types(TypeContext *ctx, ASTFunctionDef *func);

// API - Program validation
int check_program_types(TypeContext *ctx, ASTProgram *program);

#endif // TYPE_INFERENCE_H
