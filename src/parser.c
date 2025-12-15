#include "../include/parser.h"
#include "../include/lexer.h"
#include "../include/ast.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

// ============================================================================
// PARSER CORE
// ============================================================================

Parser *parser_create(const char *input) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer_create(input);
    parser->current = lexer_next_token(parser->lexer);
    parser->peek = lexer_next_token(parser->lexer);
    parser->error = 0;
    return parser;
}

void parser_free(Parser *parser) {
    if (parser) {
        lexer_free(parser->lexer);
        free(parser);
    }
}

void parser_error(Parser *parser, const char *message) {
    fprintf(stderr, "Parser error at %d:%d %s\n",
            parser->current.line,
            parser->current.column,
            message);
    parser->error = 1;
}

int parser_has_error(Parser *parser) {
    return parser->error;
}

static void parser_advance(Parser *parser) {
    parser->current = parser->peek;
    parser->peek = lexer_next_token(parser->lexer);
}

static int parser_match(Parser *parser, TokenType type) {
    if (parser->current.type == type) {
        parser_advance(parser);
        return 1;
    }
    return 0;
}

static int parser_check(Parser *parser, TokenType type) {
    return parser->current.type == type;
}

static void parser_expect(Parser *parser, TokenType type, const char *message) {
    if (!parser_match(parser, type)) {
        parser_error(parser, message);
    }
}

// ============================================================================
// TYPE PARSING
// ============================================================================

Type *parser_parse_type(Parser *parser) {
    TypeKind kind = TYPE_VOID;
    
    if (parser_match(parser, TOK_U8)) {
        kind = TYPE_U8;
    } else if (parser_match(parser, TOK_U16)) {
        kind = TYPE_U16;
    } else if (parser_match(parser, TOK_U32)) {
        kind = TYPE_U32;
    } else if (parser_match(parser, TOK_U64)) {
        kind = TYPE_U64;
    } else if (parser_match(parser, TOK_I8)) {
        kind = TYPE_I8;
    } else if (parser_match(parser, TOK_I16)) {
        kind = TYPE_I16;
    } else if (parser_match(parser, TOK_I32)) {
        kind = TYPE_I32;
    } else if (parser_match(parser, TOK_I64)) {
        kind = TYPE_I64;
    } else if (parser_match(parser, TOK_VOID)) {
        kind = TYPE_VOID;
    } else {
        parser_error(parser, "Expected type annotation");
        return NULL;
    }
    
    Type *type = (Type *)malloc(sizeof(Type));
    type->kind = kind;
    return type;
}

// ============================================================================
// EXPRESSION PARSING
// ============================================================================

// Forward declarations
static ASTExpr *parser_parse_expression(Parser *parser);
static ASTExpr *parser_parse_primary(Parser *parser);
static ASTExpr *parser_parse_unary(Parser *parser);
static ASTExpr *parser_parse_multiplicative(Parser *parser);
static ASTExpr *parser_parse_additive(Parser *parser);
static ASTExpr *parser_parse_shift(Parser *parser);
static ASTExpr *parser_parse_comparison(Parser *parser);
static ASTExpr *parser_parse_equality(Parser *parser);
static ASTExpr *parser_parse_bitwise_and(Parser *parser);
static ASTExpr *parser_parse_bitwise_xor(Parser *parser);
static ASTExpr *parser_parse_bitwise_or(Parser *parser);

static ASTExpr *parser_parse_primary(Parser *parser) {
    // Numbers
    if (parser_check(parser, TOK_NUMBER)) {
        uint64_t value = strtoull(parser->current.value, NULL, 0);
        parser_advance(parser);
        return ast_expr_number(value, TYPE_U32);
    }
    
    // Identifiers and function calls
    if (parser_check(parser, TOK_IDENTIFIER)) {
        const char *name = parser->current.value;
        parser_advance(parser);
        
        // Check for function call
        if (parser_check(parser, TOK_LPAREN)) {
            parser_advance(parser);
            
            // Parse arguments
            ASTExpr **args = NULL;
            size_t arg_count = 0;
            size_t arg_capacity = 10;
            args = (ASTExpr **)malloc(arg_capacity * sizeof(ASTExpr *));
            
            if (!parser_check(parser, TOK_RPAREN)) {
                while (1) {
                    if (arg_count >= arg_capacity) {
                        arg_capacity *= 2;
                        args = (ASTExpr **)realloc(args, arg_capacity * sizeof(ASTExpr *));
                    }
                    args[arg_count] = parser_parse_expression(parser);
                    arg_count++;
                    
                    if (!parser_match(parser, TOK_COMMA)) break;
                }
            }
            parser_expect(parser, TOK_RPAREN, "Expected ')' after arguments");
            
            // Regular function call
            ASTExpr *func = ast_expr_identifier(name);
            return ast_expr_call(func, args, arg_count);
        }
        
        // Simple identifier (variable reference)
        return ast_expr_identifier(name);
    }
    
    // Boolean literals
    if (parser_match(parser, TOK_TRUE)) {
        return ast_expr_true();
    }
    
    if (parser_match(parser, TOK_FALSE)) {
        return ast_expr_false();
    }
    
    // Parenthesized expression
    if (parser_check(parser, TOK_LPAREN)) {
        parser_advance(parser);
        ASTExpr *expr = parser_parse_expression(parser);
        parser_expect(parser, TOK_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    parser_error(parser, "Expected primary expression");
    return NULL;
}

static ASTExpr *parser_parse_unary(Parser *parser) {
    // Unary operators: -, !, ~ (~ is TOK_XOR in your token set)
    if (parser_match(parser, TOK_MINUS)) {
        ASTExpr *operand = parser_parse_unary(parser);
        return ast_expr_unary_op(UOP_NEG, operand);
    }
    
    if (parser_match(parser, TOK_NOT)) {
        ASTExpr *operand = parser_parse_unary(parser);
        return ast_expr_unary_op(UOP_NOT, operand);
    }
    
    if (parser_match(parser, TOK_XOR)) {
        ASTExpr *operand = parser_parse_unary(parser);
        return ast_expr_unary_op(UOP_BIT_NOT, operand);
    }
    
    return parser_parse_primary(parser);
}

static ASTExpr *parser_parse_multiplicative(Parser *parser) {
    ASTExpr *left = parser_parse_unary(parser);
    
    while (parser_check(parser, TOK_STAR) || parser_check(parser, TOK_SLASH) || parser_check(parser, TOK_PERCENT)) {
        BinaryOp op;
        if (parser_match(parser, TOK_STAR)) {
            op = BOP_MUL;
        } else if (parser_match(parser, TOK_SLASH)) {
            op = BOP_DIV;
        } else {
            parser_match(parser, TOK_PERCENT);
            op = BOP_MOD;
        }
        
        ASTExpr *right = parser_parse_unary(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_additive(Parser *parser) {
    ASTExpr *left = parser_parse_multiplicative(parser);
    
    while (parser_check(parser, TOK_PLUS) || parser_check(parser, TOK_MINUS)) {
        BinaryOp op = parser_match(parser, TOK_PLUS) ? BOP_ADD : BOP_SUB;
        if (op != BOP_ADD) parser_match(parser, TOK_MINUS);
        
        ASTExpr *right = parser_parse_multiplicative(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_shift(Parser *parser) {
    ASTExpr *left = parser_parse_additive(parser);
    
    while (parser_check(parser, TOK_LSHIFT) || parser_check(parser, TOK_RSHIFT)) {
        BinaryOp op = parser_match(parser, TOK_LSHIFT) ? BOP_LSHIFT : BOP_RSHIFT;
        if (op != BOP_LSHIFT) parser_match(parser, TOK_RSHIFT);
        
        ASTExpr *right = parser_parse_additive(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_comparison(Parser *parser) {
    ASTExpr *left = parser_parse_shift(parser);
    
    while (parser_check(parser, TOK_LT) || parser_check(parser, TOK_GT) ||
           parser_check(parser, TOK_LE) || parser_check(parser, TOK_GE)) {
        BinaryOp op;
        if (parser_match(parser, TOK_LT)) {
            op = BOP_LT;
        } else if (parser_match(parser, TOK_GT)) {
            op = BOP_GT;
        } else if (parser_match(parser, TOK_LE)) {
            op = BOP_LE;
        } else {
            parser_match(parser, TOK_GE);
            op = BOP_GE;
        }
        
        ASTExpr *right = parser_parse_shift(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_equality(Parser *parser) {
    ASTExpr *left = parser_parse_comparison(parser);
    
    while (parser_check(parser, TOK_EQ) || parser_check(parser, TOK_NE)) {
        BinaryOp op = parser_match(parser, TOK_EQ) ? BOP_EQ : BOP_NE;
        if (op != BOP_EQ) parser_match(parser, TOK_NE);
        
        ASTExpr *right = parser_parse_comparison(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_bitwise_and(Parser *parser) {
    ASTExpr *left = parser_parse_equality(parser);
    
    while (parser_match(parser, TOK_AND)) {
        ASTExpr *right = parser_parse_equality(parser);
        left = ast_expr_binary_op(BOP_AND, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_bitwise_xor(Parser *parser) {
    ASTExpr *left = parser_parse_bitwise_and(parser);
    
    while (parser_match(parser, TOK_XOR)) {
        ASTExpr *right = parser_parse_bitwise_and(parser);
        left = ast_expr_binary_op(BOP_XOR, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_bitwise_or(Parser *parser) {
    ASTExpr *left = parser_parse_bitwise_xor(parser);
    
    while (parser_match(parser, TOK_OR)) {
        ASTExpr *right = parser_parse_bitwise_xor(parser);
        left = ast_expr_binary_op(BOP_OR, left, right);
    }
    return left;
}

static ASTExpr *parser_parse_expression(Parser *parser) {
    return parser_parse_bitwise_or(parser);
}

// ============================================================================
// STATEMENT PARSING
// ============================================================================

static ASTStmt *parser_parse_statement(Parser *parser);

static ASTStmt *parser_parse_block(Parser *parser) {
    if (!parser_match(parser, TOK_LBRACE)) {
        parser_error(parser, "Expected '{'");
        return NULL;
    }
    
    ASTStmt **statements = NULL;
    size_t stmt_count = 0;
    
    while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
        statements = (ASTStmt **)realloc(statements, (stmt_count + 1) * sizeof(ASTStmt *));
        statements[stmt_count] = parser_parse_statement(parser);
        stmt_count++;
    }
    
    parser_expect(parser, TOK_RBRACE, "Expected '}'");
    return ast_stmt_block(statements, stmt_count);
}

static ASTStmt *parser_parse_statement(Parser *parser) {
    // ============= NEW: let/var declarations =============
    if (parser_check(parser, TOK_LET) || parser_check(parser, TOK_VAR)) {
        int is_mut = parser_check(parser, TOK_VAR) ? 1 : 0;
        parser_advance(parser); // consume let/var
        
        const char *name = parser->current.value;
        parser_expect(parser, TOK_IDENTIFIER, "Expected variable name");
        
        parser_expect(parser, TOK_COLON, "Expected ':' after variable name");
        
        Type *decl_type = parser_parse_type(parser);
        if (!decl_type) {
            return NULL;
        }
        
        // Require '=' for initialization (both let and var)
        if (!(parser_check(parser, TOK_ASSIGN) || parser_check(parser, TOK_EQUAL))) {
            parser_error(parser, "Expected '=' in variable declaration");
            return NULL;
        }
        parser_advance(parser); // consume = or ==
        
        ASTExpr *init = parser_parse_expression(parser);
        
        // Optional semicolon
        parser_match(parser, TOK_SEMICOLON);
        
        return ast_stmt_var_decl(name, decl_type, init, is_mut);
    }
    
    // Return statement
    if (parser_match(parser, TOK_RETURN)) {
        ASTExpr *value = NULL;
        if (!parser_check(parser, TOK_SEMICOLON) &&
            !parser_check(parser, TOK_RBRACE) &&
            !parser_check(parser, TOK_EOF)) {
            value = parser_parse_expression(parser);
        }
        parser_match(parser, TOK_SEMICOLON);
        return ast_stmt_return(value);
    }
    
    // Block
    if (parser_check(parser, TOK_LBRACE)) {
        return parser_parse_block(parser);
    }
    
    // Expression statement
    ASTExpr *expr = parser_parse_expression(parser);
    parser_match(parser, TOK_SEMICOLON);
    return ast_stmt_expr(expr);
}

// ============================================================================
// FUNCTION PARSING - SUPPORTS proc, func, AND fn
// ============================================================================

ASTFunctionDef *parser_parse_function(Parser *parser) {
    // Handle Nim-style (proc/func) or deprecated (fn) keywords
    int is_nim_style = 0;
    
    if (parser_match(parser, TOK_PROC)) {
        is_nim_style = 1;
    } else if (parser_match(parser, TOK_FUNC)) {
        is_nim_style = 1;
    } else if (parser_match(parser, TOK_FN)) {
        is_nim_style = 0;
    } else {
        parser_error(parser, "Expected 'proc', 'func', or 'fn'");
        return NULL;
    }
    
    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected function name");
    
    parser_expect(parser, TOK_LPAREN, "Expected '('");
    parser_expect(parser, TOK_RPAREN, "Expected ')'");
    
    Type *return_type = (Type *)malloc(sizeof(Type));
    return_type->kind = TYPE_U32;
    
    // Nim-style: expects ":" then type
    // fn-style: expects "->" then type
    if (is_nim_style) {
        parser_expect(parser, TOK_COLON, "Expected ':' after ()");
        Type *parsed_type = parser_parse_type(parser);
        if (parsed_type) {
            return_type->kind = parsed_type->kind;
            free(parsed_type);
        }
    } else {
        // fn-style: always expect "->" return type
        parser_expect(parser, TOK_ARROW, "Expected '->' after ()");
        Type *parsed_type = parser_parse_type(parser);
        if (parsed_type) {
            return_type->kind = parsed_type->kind;
            free(parsed_type);
        }
    }
    
    // Both accept "=" or "{" to start body
    ASTStmt *body = NULL;
    if (parser_match(parser, TOK_ASSIGN)) {
        // Single statement after =
        ASTStmt **statements = (ASTStmt **)malloc(sizeof(ASTStmt *));
        statements[0] = parser_parse_statement(parser);
        body = ast_stmt_block(statements, 1);
    } else if (parser_check(parser, TOK_LBRACE)) {
        // Block with braces
        body = parser_parse_block(parser);
    } else {
        parser_error(parser, "Expected '=' or '{' for function body");
        body = ast_stmt_block(NULL, 0);
    }

    
    return ast_function_def(name, return_type, NULL, NULL, 0, body);
}

// ============================================================================
// PERIPHERAL PARSING - DSL SUPPORT
// ============================================================================

static ASTField *parser_parse_field(Parser *parser) {
    parser_expect(parser, TOK_FIELD, "Expected 'field'");
    
    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected field name");
    
    parser_expect(parser, TOK_COLON, "Expected ':' after field name");
    parser_expect(parser, TOK_LBRACKET, "Expected '[' for bit range");
    
    uint32_t start = strtoull(parser->current.value, NULL, 0);
    parser_expect(parser, TOK_NUMBER, "Expected start bit");
    
    parser_expect(parser, TOK_COLON, "Expected ':' in bit range");
    
    uint32_t end = strtoull(parser->current.value, NULL, 0);
    parser_expect(parser, TOK_NUMBER, "Expected end bit");
    
    parser_expect(parser, TOK_RBRACKET, "Expected ']' after bit range");
    
    // Parse access specifier
    AccessKind access = ACCESS_RW;
    if (parser_match(parser, TOK_RO)) {
        access = ACCESS_RO;
    } else if (parser_match(parser, TOK_WO)) {
        access = ACCESS_WO;
    } else if (parser_match(parser, TOK_RW)) {
        access = ACCESS_RW;
    } else if (parser_match(parser, TOK_W1C)) {
        access = ACCESS_W1C;
    }

    
    parser_match(parser, TOK_SEMICOLON);
    
    return ast_field_create(name, start, end, access);
}

static ASTRegister *parser_parse_register(Parser *parser) {
    parser_expect(parser, TOK_REGISTER, "Expected 'register'");
    
    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected register name");
    
    parser_expect(parser, TOK_COLON, "Expected ':' after register name");
    
    // Parse type
    TypeKind kind = TYPE_U32;
    if (parser_match(parser, TOK_U8)) {
        kind = TYPE_U8;
    } else if (parser_match(parser, TOK_U16)) {
        kind = TYPE_U16;
    } else if (parser_match(parser, TOK_U32)) {
        kind = TYPE_U32;
    } else if (parser_match(parser, TOK_U64)) {
        kind = TYPE_U64;
    }
    Type *type = (Type *)malloc(sizeof(Type));
    type->kind = kind;
    
    parser_expect(parser, TOK_AT, "Expected '@' before offset");
    
    uint32_t offset = strtoull(parser->current.value, NULL, 0);
    parser_expect(parser, TOK_NUMBER, "Expected offset value");
    
    parser_expect(parser, TOK_LBRACE, "Expected '{' after register");
    
    ASTRegister *reg = ast_register_create(name, type, offset);
    
    // Parse fields
    while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
        ASTField *field = parser_parse_field(parser);
        if (field) {
            ast_register_add_field(reg, field);
        }
    }
    
    parser_expect(parser, TOK_RBRACE, "Expected '}' after register");
    
    return reg;
}

static ASTPeripheral *parser_parse_peripheral(Parser *parser) {
    parser_expect(parser, TOK_PERIPHERAL, "Expected 'peripheral'");
    
    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected peripheral name");
    
    parser_expect(parser, TOK_AT, "Expected '@' before base address");
    
    uint32_t base = strtoull(parser->current.value, NULL, 0);
    parser_expect(parser, TOK_NUMBER, "Expected base address");
    
    parser_expect(parser, TOK_LBRACE, "Expected '{' after peripheral");
    
    ASTPeripheral *periph = ast_peripheral_create(name, base);
    
    // Parse registers
    while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
        ASTRegister *reg = parser_parse_register(parser);
        if (reg) {
            ast_peripheral_add_register(periph, reg);
        }
    }
    
    parser_expect(parser, TOK_RBRACE, "Expected '}' after peripheral");
    
    return periph;
}

// ============================================================================
// PROGRAM PARSING
// ============================================================================

ASTProgram *parser_parse_program(Parser *parser) {
    ASTProgram *program = ast_program_create();
    
    while (!parser_check(parser, TOK_EOF)) {
        // Check for function definitions (proc, func, or fn)
        if (parser_check(parser, TOK_PROC) || 
            parser_check(parser, TOK_FUNC) || 
            parser_check(parser, TOK_FN)) {
            ASTFunctionDef *func = parser_parse_function(parser);
            if (func) {
                ast_program_add_function(program, func);
            }
        }
        // Check for peripheral definitions
        else if (parser_check(parser, TOK_PERIPHERAL)) {
            ASTPeripheral *periph = parser_parse_peripheral(parser);
            if (periph) {
                ast_program_add_peripheral(program, periph);
            }
        }
        else {
            parser_error(parser, "Expected function or peripheral definition");
            break;
        }
    }
    
    return program;
}
