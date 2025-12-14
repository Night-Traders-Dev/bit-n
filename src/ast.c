#include "ast.h"
#include <stdlib.h>

ASTExpr *ast_expr_number(uint64_t value, TypeKind type) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_NUMBER;
    expr->data.number_value = value;
    return expr;
}

ASTExpr *ast_expr_identifier(const char *name) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_IDENTIFIER;
    expr->data.identifier = name;
    return expr;
}

ASTExpr *ast_expr_string(const char *value) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_STRING;
    expr->data.string_value = value;
    return expr;
}

ASTExpr *ast_expr_true(void) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BOOLEAN;
    expr->data.boolean_value = 1;
    return expr;
}

ASTExpr *ast_expr_false(void) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BOOLEAN;
    expr->data.boolean_value = 0;
    return expr;
}

ASTExpr *ast_expr_binary_op(BinaryOp op, ASTExpr *left, ASTExpr *right) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BINARY_OP;
    expr->data.binary.op = op;
    expr->data.binary.left = left;
    expr->data.binary.right = right;
    return expr;
}

ASTExpr *ast_expr_unary_op(UnaryOp op, ASTExpr *operand) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_UNARY_OP;
    expr->data.unary.op = op;
    expr->data.unary.operand = operand;
    return expr;
}

ASTExpr *ast_expr_call(ASTExpr *func, ASTExpr **args, size_t arg_count) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_CALL;
    expr->data.call.func = func;
    expr->data.call.args = args;
    expr->data.call.arg_count = arg_count;
    return expr;
}

ASTExpr *ast_expr_array_index(ASTExpr *array, ASTExpr *index) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_ARRAY_INDEX;
    expr->data.array_access.array = array;
    expr->data.array_access.index = index;
    return expr;
}

ASTExpr *ast_expr_bit_slice(ASTExpr *expr_node, uint32_t start, uint32_t end) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_BIT_SLICE;
    expr->data.bit_slice.expr = expr_node;
    expr->data.bit_slice.start = start;
    expr->data.bit_slice.end = end;
    return expr;
}

ASTExpr *ast_expr_member_access(ASTExpr *object, const char *field) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_MEMBER_ACCESS;
    expr->data.member.object = object;
    expr->data.member.field = field;
    return expr;
}

void ast_free_expr(ASTExpr *expr) { if (!expr) return; free(expr); }

ASTStmt *ast_stmt_var_decl(const char *name, Type *type, ASTExpr *init, int is_mut) {
    ASTStmt *stmt = malloc(sizeof(ASTStmt));
    stmt->kind = STMT_VAR_DECL;
    stmt->data.var_decl.name = name;
    stmt->data.var_decl.type = type;
    stmt->data.var_decl.init = init;
    stmt->data.var_decl.is_mut = is_mut;
    return stmt;
}

ASTStmt *ast_stmt_expr(ASTExpr *expr) {
    ASTStmt *stmt = malloc(sizeof(ASTStmt));
    stmt->kind = STMT_EXPR;
    stmt->data.expr_stmt.expr = expr;
    return stmt;
}

ASTStmt *ast_stmt_return(ASTExpr *value) {
    ASTStmt *stmt = malloc(sizeof(ASTStmt));
    stmt->kind = STMT_RETURN;
    stmt->data.ret.value = value;
    return stmt;
}

ASTStmt *ast_stmt_block(ASTStmt **statements, size_t count) {
    ASTStmt *stmt = malloc(sizeof(ASTStmt));
    stmt->kind = STMT_BLOCK;
    stmt->data.block.statements = statements;
    stmt->data.block.count = count;
    return stmt;
}

void ast_free_stmt(ASTStmt *stmt) { if (!stmt) return; free(stmt); }

ASTFunctionDef *ast_function_def(const char *name, Type *return_type, 
                                  char **param_names, Type **param_types,
                                  size_t param_count, ASTStmt *body) {
    ASTFunctionDef *func = malloc(sizeof(ASTFunctionDef));
    func->name = name;
    func->return_type = return_type;
    func->param_names = param_names;
    func->param_types = param_types;
    func->param_count = param_count;
    func->body = body;
    return func;
}

ASTProgram *ast_program_create(void) {
    ASTProgram *prog = malloc(sizeof(ASTProgram));
    prog->functions = NULL;
    prog->function_count = 0;
    return prog;
}

void ast_program_add_function(ASTProgram *prog, ASTFunctionDef *func) {
    prog->functions = realloc(prog->functions, (prog->function_count + 1) * sizeof(ASTFunctionDef *));
    prog->functions[prog->function_count++] = func;
}

void ast_free_program(ASTProgram *prog) { if (!prog) return; free(prog->functions); free(prog); }
