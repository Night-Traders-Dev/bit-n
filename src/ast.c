#include "../include/ast.h"
#include <stdlib.h>
#include <string.h>

// ============================================================================
// Expression constructors
// ============================================================================

ASTExpr *ast_expr_number(uint64_t value, TypeKind type) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_NUMBER;
    expr->line = 0;
    expr->data.number_value = value;
    return expr;
}

ASTExpr *ast_expr_identifier(const char *name) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_IDENTIFIER;
    expr->line = 0;
    expr->data.identifier = name;
    return expr;
}

ASTExpr *ast_expr_string(const char *value) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_STRING;
    expr->line = 0;
    expr->data.string_value = value;
    return expr;
}

ASTExpr *ast_expr_true(void) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BOOLEAN;
    expr->line = 0;
    expr->data.boolean_value = 1;
    return expr;
}

ASTExpr *ast_expr_false(void) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BOOLEAN;
    expr->line = 0;
    expr->data.boolean_value = 0;
    return expr;
}

ASTExpr *ast_expr_binary_op(BinaryOp op, ASTExpr *left, ASTExpr *right) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BINARY_OP;
    expr->line = 0;
    expr->data.binary.op = op;
    expr->data.binary.left = left;
    expr->data.binary.right = right;
    return expr;
}

ASTExpr *ast_expr_unary_op(UnaryOp op, ASTExpr *operand) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_UNARY_OP;
    expr->line = 0;
    expr->data.unary.op = op;
    expr->data.unary.operand = operand;
    return expr;
}

ASTExpr *ast_expr_call(ASTExpr *func, ASTExpr **args, size_t arg_count) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_CALL;
    expr->line = 0;
    expr->data.call.func = func;
    expr->data.call.args = args;
    expr->data.call.arg_count = arg_count;
    return expr;
}

ASTExpr *ast_expr_array_index(ASTExpr *array, ASTExpr *index) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_ARRAY_INDEX;
    expr->line = 0;
    expr->data.array_access.array = array;
    expr->data.array_access.index = index;
    return expr;
}

ASTExpr *ast_expr_bit_slice(ASTExpr *expr_inner, uint32_t start, uint32_t end) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BIT_SLICE;
    expr->line = 0;
    expr->data.bit_slice.expr = expr_inner;
    expr->data.bit_slice.start = start;
    expr->data.bit_slice.end = end;
    return expr;
}

ASTExpr *ast_expr_member_access(ASTExpr *object, const char *field) {
    ASTExpr *expr = (ASTExpr *)malloc(sizeof(ASTExpr));
    expr->kind = EXPR_MEMBER_ACCESS;
    expr->line = 0;
    expr->data.member.object = object;
    expr->data.member.field = field;
    return expr;
}

void ast_free_expr(ASTExpr *expr) {
    if (!expr) return;
    switch (expr->kind) {
        case EXPR_BINARY_OP:
            ast_free_expr(expr->data.binary.left);
            ast_free_expr(expr->data.binary.right);
            break;
        case EXPR_UNARY_OP:
            ast_free_expr(expr->data.unary.operand);
            break;
        case EXPR_CALL:
            ast_free_expr(expr->data.call.func);
            for (size_t i = 0; i < expr->data.call.arg_count; i++) {
                ast_free_expr(expr->data.call.args[i]);
            }
            free(expr->data.call.args);
            break;
        case EXPR_ARRAY_INDEX:
            ast_free_expr(expr->data.array_access.array);
            ast_free_expr(expr->data.array_access.index);
            break;
        case EXPR_BIT_SLICE:
            ast_free_expr(expr->data.bit_slice.expr);
            break;
        case EXPR_MEMBER_ACCESS:
            ast_free_expr(expr->data.member.object);
            break;
        default:
            break;
    }
    free(expr);
}

// ============================================================================
// Statement constructors
// ============================================================================

ASTStmt *ast_stmt_var_decl(const char *name, Type *type, ASTExpr *init, int is_mut) {
    ASTStmt *stmt = (ASTStmt *)malloc(sizeof(ASTStmt));
    stmt->kind = STMT_VAR_DECL;
    stmt->data.var_decl.name = name;
    stmt->data.var_decl.type = type;
    stmt->data.var_decl.init = init;
    stmt->data.var_decl.is_mut = is_mut;
    return stmt;
}

ASTStmt *ast_stmt_expr(ASTExpr *expr) {
    ASTStmt *stmt = (ASTStmt *)malloc(sizeof(ASTStmt));
    stmt->kind = STMT_EXPR;
    stmt->data.expr_stmt.expr = expr;
    return stmt;
}

ASTStmt *ast_stmt_return(ASTExpr *value) {
    ASTStmt *stmt = (ASTStmt *)malloc(sizeof(ASTStmt));
    stmt->kind = STMT_RETURN;
    stmt->data.ret.value = value;
    return stmt;
}

ASTStmt *ast_stmt_block(ASTStmt **statements, size_t count) {
    ASTStmt *stmt = (ASTStmt *)malloc(sizeof(ASTStmt));
    stmt->kind = STMT_BLOCK;
    stmt->data.block.statements = statements;
    stmt->data.block.count = count;
    return stmt;
}

void ast_free_stmt(ASTStmt *stmt) {
    if (!stmt) return;
    switch (stmt->kind) {
        case STMT_VAR_DECL:
            ast_free_expr(stmt->data.var_decl.init);
            if (stmt->data.var_decl.type) free(stmt->data.var_decl.type);
            break;
        case STMT_EXPR:
            ast_free_expr(stmt->data.expr_stmt.expr);
            break;
        case STMT_RETURN:
            ast_free_expr(stmt->data.ret.value);
            break;
        case STMT_BLOCK:
            for (size_t i = 0; i < stmt->data.block.count; i++) {
                ast_free_stmt(stmt->data.block.statements[i]);
            }
            free(stmt->data.block.statements);
            break;
        default:
            break;
    }
    free(stmt);
}

// ============================================================================
// Function constructor
// ============================================================================

ASTFunctionDef *ast_function_def(const char *name, Type *return_type,
                                 char **param_names, Type **param_types,
                                 size_t param_count, ASTStmt *body) {
    ASTFunctionDef *func = (ASTFunctionDef *)malloc(sizeof(ASTFunctionDef));
    func->name = name;
    func->return_type = return_type;
    func->param_names = param_names;
    func->param_types = param_types;
    func->param_count = param_count;
    func->body = body;
    return func;
}

// ============================================================================
// DSL - Peripheral constructors
// ============================================================================

ASTField *ast_field_create(const char *name, uint32_t start, uint32_t end, AccessKind access) {
    ASTField *field = (ASTField *)malloc(sizeof(ASTField));
    field->name = name;
    field->start_bit = start;
    field->end_bit = end;
    field->access = access;
    return field;
}

void ast_field_free(ASTField *field) {
    if (field) free(field);
}

ASTRegister *ast_register_create(const char *name, Type *type, uint32_t offset) {
    ASTRegister *reg = (ASTRegister *)malloc(sizeof(ASTRegister));
    reg->name = name;
    reg->type = type;
    reg->offset = offset;
    reg->fields = NULL;
    reg->field_count = 0;
    return reg;
}

void ast_register_add_field(ASTRegister *reg, ASTField *field) {
    reg->fields = (ASTField **)realloc(reg->fields, (reg->field_count + 1) * sizeof(ASTField *));
    reg->fields[reg->field_count++] = field;
}

void ast_register_free(ASTRegister *reg) {
    if (!reg) return;
    for (size_t i = 0; i < reg->field_count; i++) {
        ast_field_free(reg->fields[i]);
    }
    free(reg->fields);
    if (reg->type) free(reg->type);
    free(reg);
}

ASTPeripheral *ast_peripheral_create(const char *name, uint32_t base_address) {
    ASTPeripheral *periph = (ASTPeripheral *)malloc(sizeof(ASTPeripheral));
    periph->name = name;
    periph->base_address = base_address;
    periph->registers = NULL;
    periph->register_count = 0;
    return periph;
}

void ast_peripheral_add_register(ASTPeripheral *periph, ASTRegister *reg) {
    periph->registers = (ASTRegister **)realloc(periph->registers,
                                                 (periph->register_count + 1) * sizeof(ASTRegister *));
    periph->registers[periph->register_count++] = reg;
}

void ast_peripheral_free(ASTPeripheral *periph) {
    if (!periph) return;
    for (size_t i = 0; i < periph->register_count; i++) {
        ast_register_free(periph->registers[i]);
    }
    free(periph->registers);
    free(periph);
}

// ============================================================================
// Program constructors
// ============================================================================

ASTProgram *ast_program_create(void) {
    ASTProgram *prog = (ASTProgram *)malloc(sizeof(ASTProgram));
    prog->functions = NULL;
    prog->function_count = 0;
    prog->peripherals = NULL;
    prog->peripheral_count = 0;
    return prog;
}

void ast_program_add_function(ASTProgram *prog, ASTFunctionDef *func) {
    prog->functions = (ASTFunctionDef **)realloc(prog->functions,
                                                  (prog->function_count + 1) * sizeof(ASTFunctionDef *));
    prog->functions[prog->function_count++] = func;
}

void ast_program_add_peripheral(ASTProgram *prog, ASTPeripheral *periph) {
    prog->peripherals = (ASTPeripheral **)realloc(prog->peripherals,
                                                   (prog->peripheral_count + 1) * sizeof(ASTPeripheral *));
    prog->peripherals[prog->peripheral_count++] = periph;
}

void ast_free_program(ASTProgram *prog) {
    if (!prog) return;
    
    for (size_t i = 0; i < prog->function_count; i++) {
        if (prog->functions[i]) {
            ast_free_stmt(prog->functions[i]->body);
            free(prog->functions[i]);
        }
    }
    free(prog->functions);
    
    for (size_t i = 0; i < prog->peripheral_count; i++) {
        ast_peripheral_free(prog->peripherals[i]);
    }
    free(prog->peripherals);
    
    free(prog);
}
