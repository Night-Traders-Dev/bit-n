#include "type_inference.h"
#include "type_system.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// Create type context
TypeContext *type_context_create(void) {
    TypeContext *ctx = (TypeContext *)malloc(sizeof(TypeContext));
    ctx->symbols = symbol_table_create();
    ctx->error_count = 0;
    ctx->current_function = NULL;
    ctx->expected_return_type = NULL;
    return ctx;
}

// Free type context
void type_context_free(TypeContext *ctx) {
    if (ctx) {
        if (ctx->symbols) {
            symbol_table_free(ctx->symbols);
        }
        if (ctx->expected_return_type) {
            type_free(ctx->expected_return_type);
        }
        free(ctx);
    }
}

// Set current function context
void type_context_set_function(TypeContext *ctx, const char *name, Type *return_type) {
    if (!ctx) return;
    
    ctx->current_function = name;
    if (ctx->expected_return_type) {
        type_free(ctx->expected_return_type);
    }
    ctx->expected_return_type = type_clone(return_type);
}

// Forward declaration
static Type *infer_binary_op_type(TypeContext *ctx, BinaryOp op, Type *left, Type *right);

// Infer type of an expression
Type *infer_expr_type(TypeContext *ctx, ASTExpr *expr) {
    if (!ctx || !expr) {
        return NULL;
    }
    
    switch (expr->kind) {
        case EXPR_NUMBER: {
            // Numbers default to u32
            return type_from_kind(TYPE_U32);
        }
        
        case EXPR_IDENTIFIER: {
            // Look up variable in symbol table
            Symbol *sym = symbol_table_lookup(ctx->symbols, expr->data.identifier);
            if (!sym) {
                fprintf(stderr, "Error: undefined variable '%s'\n", expr->data.identifier);
                ctx->error_count++;
                return NULL;
            }
            return type_clone(sym->type);
        }
        
        case EXPR_BOOLEAN: {
            // Booleans are u8 (0 or 1)
            return type_from_kind(TYPE_U8);
        }
        
        case EXPR_STRING: {
            // String type - not fully defined yet
            // For now, treat as error
            fprintf(stderr, "Error: string literals not yet supported\n");
            ctx->error_count++;
            return NULL;
        }
        
        case EXPR_UNARY_OP: {
            // Unary operations preserve type
            Type *operand_type = infer_expr_type(ctx, expr->data.unary.operand);
            if (!operand_type) {
                return NULL;
            }
            
            switch (expr->data.unary.op) {
                case UOP_NOT:
                case UOP_BIT_NOT:
                case UOP_NEG:
                    // Result type is same as operand for bitwise ops
                    // For logical NOT, should return bool but returning operand type for now
                    return operand_type;
                default:
                    type_free(operand_type);
                    return NULL;
            }
        }
        
        case EXPR_BINARY_OP: {
            Type *left = infer_expr_type(ctx, expr->data.binary.left);
            if (!left) {
                return NULL;
            }
            
            Type *right = infer_expr_type(ctx, expr->data.binary.right);
            if (!right) {
                type_free(left);
                return NULL;
            }
            
            // Check type compatibility
            if (!type_compatible(left, right)) {
                fprintf(stderr, "Error: type mismatch in binary operation\n");
                fprintf(stderr, "  Left type: %s\n", type_to_string(left));
                fprintf(stderr, "  Right type: %s\n", type_to_string(right));
                ctx->error_count++;
                type_free(left);
                type_free(right);
                return NULL;
            }
            
            // Determine result type
            Type *result = infer_binary_op_type(ctx, expr->data.binary.op, left, right);
            type_free(left);
            type_free(right);
            return result;
        }
        
        case EXPR_BIT_SLICE: {
            Type *obj_type = infer_expr_type(ctx, expr->data.bit_slice.expr);
            if (!obj_type) {
                return NULL;
            }
            
            // Verify operand is integer type
            if (!type_is_integer(obj_type)) {
                fprintf(stderr, "Error: bit slice requires integer type, got %s\n", 
                        type_to_string(obj_type));
                ctx->error_count++;
                type_free(obj_type);
                return NULL;
            }
            
            // Determine result type based on slice width
            uint32_t width = expr->data.bit_slice.end - expr->data.bit_slice.start;
            
            if (width <= 0) {
                fprintf(stderr, "Error: invalid bit slice range [%u:%u]\n",
                        expr->data.bit_slice.start, expr->data.bit_slice.end);
                ctx->error_count++;
                type_free(obj_type);
                return NULL;
            }
            
            type_free(obj_type);
            
            // Determine smallest type that fits
            if (width <= 8) {
                return type_from_kind(TYPE_U8);
            } else if (width <= 16) {
                return type_from_kind(TYPE_U16);
            } else if (width <= 32) {
                return type_from_kind(TYPE_U32);
            } else {
                return type_from_kind(TYPE_U64);
            }
        }
        
        case EXPR_ARRAY_INDEX: {
            // Array indexing - not fully defined
            fprintf(stderr, "Error: array indexing not yet supported\n");
            ctx->error_count++;
            return NULL;
        }
        
        case EXPR_CALL: {
            // Function calls - not fully defined
            fprintf(stderr, "Error: function calls not yet supported in expressions\n");
            ctx->error_count++;
            return NULL;
        }
        
        case EXPR_MEMBER_ACCESS: {
            // Member access - not fully defined
            fprintf(stderr, "Error: member access not yet supported\n");
            ctx->error_count++;
            return NULL;
        }
        
        default:
            return NULL;
    }
}

// Determine result type of binary operation
static Type *infer_binary_op_type(TypeContext *ctx, BinaryOp op, Type *left, Type *right) {
    (void)ctx;  // Unused in simple case
    (void)right; // Types must match, so right type not needed
    
    switch (op) {
        // Arithmetic operations preserve type
        case BOP_ADD:
        case BOP_SUB:
        case BOP_MUL:
        case BOP_DIV:
        case BOP_MOD:
            return type_clone(left);
        
        // Bitwise operations preserve type
        case BOP_AND:
        case BOP_OR:
        case BOP_XOR:
        case BOP_LSHIFT:
        case BOP_RSHIFT:
        case BOP_LROTATE:
        case BOP_RROTATE:
            return type_clone(left);
        
        // Comparison operations return u8 (boolean)
        case BOP_EQ:
        case BOP_NE:
        case BOP_LT:
        case BOP_GT:
        case BOP_LE:
        case BOP_GE:
            return type_from_kind(TYPE_U8);
        
        default:
            return type_clone(left);
    }
}

// Check types in a statement
int check_stmt_types(TypeContext *ctx, ASTStmt *stmt) {
    if (!ctx || !stmt) {
        return 1;  // OK
    }
    
    switch (stmt->kind) {
        case STMT_VAR_DECL: {
            // Check initializer type matches declaration
            if (stmt->data.var_decl.init) {
                Type *init_type = infer_expr_type(ctx, stmt->data.var_decl.init);
                if (!init_type) {
                    ctx->error_count++;
                    return 0;
                }
                
                if (!type_compatible(stmt->data.var_decl.type, init_type)) {
                    fprintf(stderr, "Error: initializer type mismatch\n");
                    fprintf(stderr, "  Variable type: %s\n", type_to_string(stmt->data.var_decl.type));
                    fprintf(stderr, "  Initializer type: %s\n", type_to_string(init_type));
                    ctx->error_count++;
                    type_free(init_type);
                    return 0;
                }
                type_free(init_type);
            }
            
            // Add variable to symbol table
            Type *var_type = type_clone(stmt->data.var_decl.type);
            if (!symbol_table_add_symbol(ctx->symbols, 
                                        stmt->data.var_decl.name,
                                        var_type,
                                        0)) {  // Not a parameter
                ctx->error_count++;
                return 0;
            }
            return 1;
        }
        
        case STMT_EXPR: {
            Type *expr_type = infer_expr_type(ctx, stmt->data.expr_stmt.expr);
            if (!expr_type) {
                ctx->error_count++;
                return 0;
            }
            type_free(expr_type);
            return 1;
        }
        
        case STMT_RETURN: {
            if (stmt->data.ret.value) {
                Type *ret_type = infer_expr_type(ctx, stmt->data.ret.value);
                if (!ret_type) {
                    ctx->error_count++;
                    return 0;
                }
                
                // Check return type matches function signature
                if (ctx->expected_return_type) {
                    if (!type_compatible(ctx->expected_return_type, ret_type)) {
                        fprintf(stderr, "Error: return type mismatch\n");
                        fprintf(stderr, "  Expected: %s\n", type_to_string(ctx->expected_return_type));
                        fprintf(stderr, "  Got: %s\n", type_to_string(ret_type));
                        ctx->error_count++;
                        type_free(ret_type);
                        return 0;
                    }
                }
                type_free(ret_type);
            }
            return 1;
        }
        
        case STMT_BLOCK: {
            // Push new scope for block
            symbol_table_push_scope(ctx->symbols);
            
            int result = 1;
            for (size_t i = 0; i < stmt->data.block.count; i++) {
                if (!check_stmt_types(ctx, stmt->data.block.statements[i])) {
                    result = 0;
                }
            }
            
            // Pop scope when exiting block
            symbol_table_pop_scope(ctx->symbols);
            return result;
        }
        
        case STMT_IF: {
            // TODO: IF statement handling
            return 1;
        }
        
        case STMT_WHILE: {
            // TODO: WHILE statement handling
            return 1;
        }
        
        default:
            return 1;
    }
}

// Check types in a function
int check_function_types(TypeContext *ctx, ASTFunctionDef *func) {
    if (!ctx || !func) {
        return 1;
    }
    
    // Push function scope
    symbol_table_push_scope(ctx->symbols);
    
    // Set function context
    type_context_set_function(ctx, func->name, func->return_type);
    
    // Add parameters to symbol table
    for (size_t i = 0; i < func->param_count; i++) {
        Type *param_type = type_clone(func->param_types[i]);
        if (!symbol_table_add_symbol(ctx->symbols,
                                    func->param_names[i],
                                    param_type,
                                    1)) {  // Is parameter
            ctx->error_count++;
            symbol_table_pop_scope(ctx->symbols);
            return 0;
        }
    }
    
    // Check function body
    int result = check_stmt_types(ctx, func->body);
    
    // Pop function scope
    symbol_table_pop_scope(ctx->symbols);
    
    return result;
}

// Check types in entire program
int check_program_types(TypeContext *ctx, ASTProgram *program) {
    if (!ctx || !program) {
        return 1;
    }
    
    int errors = 0;
    
    // First pass: register all functions in global scope
    for (size_t i = 0; i < program->function_count; i++) {
        ASTFunctionDef *func = program->functions[i];
        Type *func_type = type_clone(func->return_type);
        
        // Add function to symbol table (as a special marker)
        // For now, just mark that function is defined
        if (!symbol_table_add_symbol(ctx->symbols, func->name, func_type, 0)) {
            errors++;
        }
    }
    
    // Second pass: check each function
    for (size_t i = 0; i < program->function_count; i++) {
        if (!check_function_types(ctx, program->functions[i])) {
            errors++;
        }
    }
    
    return errors == 0;
}
