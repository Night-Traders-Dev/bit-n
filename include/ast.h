#ifndef BITN_AST_H
#define BITN_AST_H

#include <stdint.h>
#include <stddef.h>

// ============================================================================
// Types
// ============================================================================

typedef enum {
    TYPE_VOID, TYPE_U8, TYPE_U16, TYPE_U32, TYPE_U64, TYPE_I8, TYPE_I16, TYPE_I32, TYPE_I64,
} TypeKind;

typedef struct { TypeKind kind; } Type;

// ============================================================================
// Expressions
// ============================================================================

typedef enum {
    EXPR_NUMBER, EXPR_IDENTIFIER, EXPR_STRING, EXPR_BOOLEAN,
    EXPR_BINARY_OP, EXPR_UNARY_OP, EXPR_CALL, EXPR_ARRAY_INDEX,
    EXPR_BIT_SLICE, EXPR_MEMBER_ACCESS,
} ExprKind;

typedef enum {
    BOP_ADD, BOP_SUB, BOP_MUL, BOP_DIV, BOP_MOD,
    BOP_AND, BOP_OR, BOP_XOR,
    BOP_LSHIFT, BOP_RSHIFT, BOP_LROTATE, BOP_RROTATE,
    BOP_EQ, BOP_NE, BOP_LT, BOP_GT, BOP_LE, BOP_GE,
} BinaryOp;

typedef enum { UOP_NOT, UOP_BIT_NOT, UOP_NEG, } UnaryOp;

typedef struct ASTExpr {
    ExprKind kind;
    int line;
    union {
        uint64_t number_value;
        const char *identifier;
        const char *string_value;
        int boolean_value;
        struct { BinaryOp op; struct ASTExpr *left; struct ASTExpr *right; } binary;
        struct { UnaryOp op; struct ASTExpr *operand; } unary;
        struct { struct ASTExpr *func; struct ASTExpr **args; size_t arg_count; } call;
        struct { struct ASTExpr *array; struct ASTExpr *index; } array_access;
        struct { struct ASTExpr *expr; uint32_t start; uint32_t end; } bit_slice;
        struct { struct ASTExpr *object; const char *field; } member;
    } data;
} ASTExpr;

ASTExpr *ast_expr_number(uint64_t value, TypeKind type);
ASTExpr *ast_expr_identifier(const char *name);
ASTExpr *ast_expr_string(const char *value);
ASTExpr *ast_expr_true(void);
ASTExpr *ast_expr_false(void);
ASTExpr *ast_expr_binary_op(BinaryOp op, ASTExpr *left, ASTExpr *right);
ASTExpr *ast_expr_unary_op(UnaryOp op, ASTExpr *operand);
ASTExpr *ast_expr_call(ASTExpr *func, ASTExpr **args, size_t arg_count);
ASTExpr *ast_expr_array_index(ASTExpr *array, ASTExpr *index);
ASTExpr *ast_expr_bit_slice(ASTExpr *expr, uint32_t start, uint32_t end);
ASTExpr *ast_expr_member_access(ASTExpr *object, const char *field);
void ast_free_expr(ASTExpr *expr);

// ============================================================================
// Statements
// ============================================================================

typedef enum { STMT_VAR_DECL, STMT_EXPR, STMT_RETURN, STMT_IF, STMT_WHILE, STMT_BLOCK, } StmtKind;

typedef struct ASTStmt {
    StmtKind kind;
    union {
        struct { const char *name; Type *type; ASTExpr *init; int is_mut; } var_decl;
        struct { ASTExpr *expr; } expr_stmt;
        struct { ASTExpr *value; } ret;
        struct { struct ASTStmt **statements; size_t count; } block;
    } data;
} ASTStmt;

ASTStmt *ast_stmt_var_decl(const char *name, Type *type, ASTExpr *init, int is_mut);
ASTStmt *ast_stmt_expr(ASTExpr *expr);
ASTStmt *ast_stmt_return(ASTExpr *value);
ASTStmt *ast_stmt_block(ASTStmt **statements, size_t count);
void ast_free_stmt(ASTStmt *stmt);

// ============================================================================
// Functions
// ============================================================================

typedef struct {
    const char *name;
    Type *return_type;
    char **param_names;
    Type **param_types;
    size_t param_count;
    ASTStmt *body;
} ASTFunctionDef;

ASTFunctionDef *ast_function_def(const char *name, Type *return_type,
                                 char **param_names, Type **param_types,
                                 size_t param_count, ASTStmt *body);

// ============================================================================
// DSL - Peripherals, Registers, Fields
// ============================================================================

typedef enum {
    ACCESS_RO,   // Read-only
    ACCESS_WO,   // Write-only
    ACCESS_RW,   // Read-write
    ACCESS_W1C,  // Write-1-to-clear
} AccessKind;

typedef struct {
    const char *name;
    uint32_t start_bit;      // Start bit (inclusive)
    uint32_t end_bit;        // End bit (exclusive, like slice notation)
    AccessKind access;
} ASTField;

typedef struct {
    const char *name;
    Type *type;              // Register type (u32, u16, etc.)
    uint32_t offset;         // Byte offset from peripheral base
    ASTField **fields;
    size_t field_count;
} ASTRegister;

typedef struct {
    const char *name;
    uint32_t base_address;
    ASTRegister **registers;
    size_t register_count;
} ASTPeripheral;

// Peripheral construction helpers
ASTField *ast_field_create(const char *name, uint32_t start, uint32_t end, AccessKind access);
void ast_field_free(ASTField *field);

ASTRegister *ast_register_create(const char *name, Type *type, uint32_t offset);
void ast_register_add_field(ASTRegister *reg, ASTField *field);
void ast_register_free(ASTRegister *reg);

ASTPeripheral *ast_peripheral_create(const char *name, uint32_t base_address);
void ast_peripheral_add_register(ASTPeripheral *periph, ASTRegister *reg);
void ast_peripheral_free(ASTPeripheral *periph);

// ============================================================================
// Program (Functions + Peripherals)
// ============================================================================

typedef struct {
    ASTFunctionDef **functions;
    size_t function_count;
    
    ASTPeripheral **peripherals;
    size_t peripheral_count;
} ASTProgram;

ASTProgram *ast_program_create(void);
void ast_program_add_function(ASTProgram *prog, ASTFunctionDef *func);
void ast_program_add_peripheral(ASTProgram *prog, ASTPeripheral *periph);
void ast_free_program(ASTProgram *prog);

#endif
