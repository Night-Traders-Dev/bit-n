#include "ast.h"
#include "type_system.h"
#include <stdlib.h>
#include <string.h>

ASTExpr *ast_expr_number(uint64_t value, TypeKind type) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_NUMBER;
    expr->data.number_value = value;
    return expr;
}

ASTExpr *ast_expr_identifier(const char *name) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_IDENTIFIER;
    // ✅ FIX: Copy the identifier string instead of storing pointer
    if (name) {
        expr->data.identifier = malloc(strlen(name) + 1);
        strcpy(expr->data.identifier, name);
    } else {
        expr->data.identifier = NULL;
    }
    return expr;
}

ASTExpr *ast_expr_string(const char *value) {
    ASTExpr *expr = malloc(sizeof(ASTExpr));
    expr->kind = EXPR_STRING;
    // ✅ Also copy string value for consistency
    if (value) {
        expr->data.string_value = malloc(strlen(value) + 1);
        strcpy(expr->data.string_value, value);
    } else {
        expr->data.string_value = NULL;
    }
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
    // ✅ Also copy field name
    if (field) {
        expr->data.member.field = malloc(strlen(field) + 1);
        strcpy(expr->data.member.field, field);
    } else {
        expr->data.member.field = NULL;
    }
    return expr;
}

void ast_free_expr(ASTExpr *expr) {
    if (!expr) return;
    
    switch (expr->kind) {
        case EXPR_IDENTIFIER:
            // ✅ Free the copied identifier string
            free((void *)expr->data.identifier);
            break;
        case EXPR_STRING:
            // ✅ Free the copied string value
            free((void *)expr->data.string_value);
            break;
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
            free((void *)expr->data.member.field);
            break;
        default:
            break;
    }
    free(expr);
}

ASTStmt *ast_stmt_var_decl(const char *name, Type *type, ASTExpr *init, int is_mut) {
    ASTStmt *stmt = malloc(sizeof(ASTStmt));
    stmt->kind = STMT_VAR_DECL;
    // ✅ Copy variable name
    if (name) {
        stmt->data.var_decl.name = malloc(strlen(name) + 1);
        strcpy((char *)stmt->data.var_decl.name, name);
    } else {
        stmt->data.var_decl.name = NULL;
    }
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

void ast_free_stmt(ASTStmt *stmt) {
    if (!stmt) return;
    
    switch (stmt->kind) {
        case STMT_VAR_DECL:
            free((void *)stmt->data.var_decl.name);
            if (stmt->data.var_decl.type) {
                type_free(stmt->data.var_decl.type);
            }
            if (stmt->data.var_decl.init) {
                ast_free_expr(stmt->data.var_decl.init);
            }
            break;
        case STMT_EXPR:
            if (stmt->data.expr_stmt.expr) {
                ast_free_expr(stmt->data.expr_stmt.expr);
            }
            break;
        case STMT_RETURN:
            if (stmt->data.ret.value) {
                ast_free_expr(stmt->data.ret.value);
            }
            break;
        case STMT_BLOCK:
            for (size_t i = 0; i < stmt->data.block.count; i++) {
                ast_free_stmt(stmt->data.block.statements[i]);
            }
            free(stmt->data.block.statements);
            break;
        case STMT_IF:
        case STMT_WHILE:
            // TODO: Handle these when implemented
            break;
        default:
            break;
    }
    free(stmt);
}

ASTFunctionDef *ast_function_def(const char *name, Type *return_type,
                                 char **param_names, Type **param_types,
                                 size_t param_count, ASTStmt *body) {
    ASTFunctionDef *func = malloc(sizeof(ASTFunctionDef));
    // ✅ Copy function name
    if (name) {
        func->name = malloc(strlen(name) + 1);
        strcpy((char *)func->name, name);
    } else {
        func->name = NULL;
    }
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
    prog->functions[prog->function_count] = func;
    prog->function_count++;
}

void ast_free_program(ASTProgram *prog) {
    if (!prog) return;
    
    for (size_t i = 0; i < prog->function_count; i++) {
        ASTFunctionDef *func = prog->functions[i];
        free((void *)func->name);
        
        if (func->return_type) {
            type_free(func->return_type);
        }
        
        // Free parameter names
        for (size_t j = 0; j < func->param_count; j++) {
            free(func->param_names[j]);
            type_free(func->param_types[j]);
        }
        free(func->param_names);
        free(func->param_types);
        
        if (func->body) {
            ast_free_stmt(func->body);
        }
        
        free(func);
    }
    
    free(prog->functions);
    free(prog);
}
