#include "../include/parser.h"
#include "../include/lexer.h"
#include "../include/ast.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

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
    if (!parser) return;
    lexer_free(parser->lexer);
    free(parser);
}

void parser_error(Parser *parser, const char *message) {
    fprintf(stderr, "Parser error at %d:%d: %s\n",
            parser->current.line, parser->current.column, message);
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
// SAFE TOKEN TO NUMBER (fixes crashes)
// ============================================================================

static uint64_t parser_token_u64(Token tok) {
    // Token.value points into source and is NOT null-terminated.
    char buf[128];
    int n = tok.length;
    if (n < 0) n = 0;
    if (n > (int)sizeof(buf) - 1) n = (int)sizeof(buf) - 1;

    memcpy(buf, tok.value, (size_t)n);
    buf[n] = '\0';

    // base=0 supports decimal, 0x..., and 0b... if lexer emits it (strtoull
    // doesn't parse 0b by default, but your examples are 0x/decimal).
    return strtoull(buf, NULL, 0);
}

static uint32_t parser_expect_u32_literal(Parser *parser) {
    if (!parser_check(parser, TOK_NUMBER)) {
        parser_error(parser, "Expected number literal");
        return 0;
    }

    uint64_t value = parser_token_u64(parser->current);
    parser_advance(parser);
    return (uint32_t)value;
}

// ============================================================================
// DSL HELPERS
// ============================================================================

static AccessKind parser_parse_access(Parser *parser) {
    if (parser_match(parser, TOK_RO)) return ACCESS_RO;
    if (parser_match(parser, TOK_WO)) return ACCESS_WO;
    if (parser_match(parser, TOK_RW)) return ACCESS_RW;
    if (parser_match(parser, TOK_W1C)) return ACCESS_W1C;

    parser_error(parser, "Expected access specifier (ro|wo|rw|w1c)");
    return ACCESS_RW;
}

static TypeKind parser_parse_type_kind(Parser *parser) {
    if (parser_match(parser, TOK_U8))  return TYPE_U8;
    if (parser_match(parser, TOK_U16)) return TYPE_U16;
    if (parser_match(parser, TOK_U32)) return TYPE_U32;
    if (parser_match(parser, TOK_U64)) return TYPE_U64;
    if (parser_match(parser, TOK_I8))  return TYPE_I8;
    if (parser_match(parser, TOK_I16)) return TYPE_I16;
    if (parser_match(parser, TOK_I32)) return TYPE_I32;
    if (parser_match(parser, TOK_I64)) return TYPE_I64;
    if (parser_match(parser, TOK_VOID)) return TYPE_VOID;

    parser_error(parser, "Expected type annotation");
    return TYPE_U32;
}

// ============================================================================
// Forward declarations
// ============================================================================

static ASTExpr *parser_parse_expression(Parser *parser);
static ASTStmt *parser_parse_statement(Parser *parser);

// ============================================================================
// Expression parsing
// ============================================================================

static ASTExpr *parser_parse_primary(Parser *parser) {
    switch (parser->current.type) {
        case TOK_NUMBER: {
            uint64_t value = parser_token_u64(parser->current);
            parser_advance(parser);
            return ast_expr_number(value, TYPE_U32);
        }

        case TOK_IDENTIFIER: {
            const char *name = parser->current.value;
            parser_advance(parser);
            return ast_expr_identifier(name);
        }

        case TOK_TRUE:
            parser_advance(parser);
            return ast_expr_true();

        case TOK_FALSE:
            parser_advance(parser);
            return ast_expr_false();

        case TOK_LPAREN: {
            parser_advance(parser);
            ASTExpr *expr = parser_parse_expression(parser);
            parser_expect(parser, TOK_RPAREN, "Expected ')'");
            return expr;
        }

        case TOK_EOF:
            parser_error(parser, "Unexpected end of file");
            return ast_expr_number(0, TYPE_U32);

        default:
            parser_error(parser, "Unexpected token in primary expression");
            parser_advance(parser);
            return ast_expr_number(0, TYPE_U32);
    }
}

static ASTExpr *parser_parse_unary(Parser *parser) {
    if (parser_check(parser, TOK_NOT)) {
        parser_advance(parser);
        return ast_expr_unary_op(UOP_NOT, parser_parse_unary(parser));
    }

    if (parser_check(parser, TOK_MINUS)) {
        parser_advance(parser);
        return ast_expr_unary_op(UOP_NEG, parser_parse_unary(parser));
    }

    return parser_parse_primary(parser);
}

static ASTExpr *parser_parse_multiplicative(Parser *parser) {
    ASTExpr *expr = parser_parse_unary(parser);

    while (parser_check(parser, TOK_STAR) || parser_check(parser, TOK_SLASH) || parser_check(parser, TOK_PERCENT)) {
        BinaryOp op = BOP_MUL;

        if (parser_check(parser, TOK_STAR)) {
            op = BOP_MUL;
        } else if (parser_check(parser, TOK_SLASH)) {
            op = BOP_DIV;
        } else {
            op = BOP_MOD;
        }

        parser_advance(parser);
        ASTExpr *right = parser_parse_unary(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }

    return expr;
}

static ASTExpr *parser_parse_additive(Parser *parser) {
    ASTExpr *expr = parser_parse_multiplicative(parser);

    while (parser_check(parser, TOK_PLUS) || parser_check(parser, TOK_MINUS)) {
        BinaryOp op = parser_check(parser, TOK_PLUS) ? BOP_ADD : BOP_SUB;
        parser_advance(parser);

        ASTExpr *right = parser_parse_multiplicative(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }

    return expr;
}

static ASTExpr *parser_parse_shift(Parser *parser) {
    ASTExpr *expr = parser_parse_additive(parser);

    while (parser_check(parser, TOK_LSHIFT) || parser_check(parser, TOK_RSHIFT)) {
        BinaryOp op = parser_check(parser, TOK_LSHIFT) ? BOP_LSHIFT : BOP_RSHIFT;
        parser_advance(parser);

        ASTExpr *right = parser_parse_additive(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }

    return expr;
}

static ASTExpr *parser_parse_comparison(Parser *parser) {
    ASTExpr *expr = parser_parse_shift(parser);

    while (parser_check(parser, TOK_LT) || parser_check(parser, TOK_GT) ||
           parser_check(parser, TOK_LE) || parser_check(parser, TOK_GE)) {
        BinaryOp op;

        if (parser_check(parser, TOK_LT)) op = BOP_LT;
        else if (parser_check(parser, TOK_GT)) op = BOP_GT;
        else if (parser_check(parser, TOK_LE)) op = BOP_LE;
        else op = BOP_GE;

        parser_advance(parser);

        ASTExpr *right = parser_parse_shift(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }

    return expr;
}

static ASTExpr *parser_parse_equality(Parser *parser) {
    ASTExpr *expr = parser_parse_comparison(parser);

    while (parser_check(parser, TOK_EQ) || parser_check(parser, TOK_NE)) {
        BinaryOp op = parser_check(parser, TOK_EQ) ? BOP_EQ : BOP_NE;
        parser_advance(parser);

        ASTExpr *right = parser_parse_comparison(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }

    return expr;
}

static ASTExpr *parser_parse_and_expr(Parser *parser) {
    ASTExpr *expr = parser_parse_equality(parser);

    while (parser_check(parser, TOK_AND)) {
        parser_advance(parser);
        ASTExpr *right = parser_parse_equality(parser);
        expr = ast_expr_binary_op(BOP_AND, expr, right);
    }

    return expr;
}

static ASTExpr *parser_parse_or_expr(Parser *parser) {
    ASTExpr *expr = parser_parse_and_expr(parser);

    while (parser_check(parser, TOK_OR)) {
        parser_advance(parser);
        ASTExpr *right = parser_parse_and_expr(parser);
        expr = ast_expr_binary_op(BOP_OR, expr, right);
    }

    return expr;
}

static ASTExpr *parser_parse_expression(Parser *parser) {
    return parser_parse_or_expr(parser);
}

// ============================================================================
// Statement parsing
// ============================================================================

static ASTStmt *parser_parse_statement(Parser *parser) {
    if (parser_match(parser, TOK_RETURN)) {
        ASTExpr *value = NULL;

        if (!parser_check(parser, TOK_SEMICOLON) &&
            !parser_check(parser, TOK_RBRACE) &&
            !parser_check(parser, TOK_EOF)) {
            value = parser_parse_expression(parser);
        }

        // Optional semicolon
        parser_match(parser, TOK_SEMICOLON);
        return ast_stmt_return(value);
    }

    if (parser_match(parser, TOK_LBRACE)) {
        ASTStmt **statements = NULL;
        size_t stmt_count = 0;

        while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
            statements = (ASTStmt **)realloc(statements, (stmt_count + 1) * sizeof(ASTStmt *));
            statements[stmt_count++] = parser_parse_statement(parser);
        }

        parser_expect(parser, TOK_RBRACE, "Expected '}'");
        return ast_stmt_block(statements, stmt_count);
    }

    ASTExpr *expr = parser_parse_expression(parser);
    parser_match(parser, TOK_SEMICOLON);
    return ast_stmt_expr(expr);
}

// ============================================================================
// DSL - Peripheral parsing
// ============================================================================

static ASTField *parser_parse_field(Parser *parser) {
    parser_expect(parser, TOK_FIELD, "Expected 'field'");

    const char *field_name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected field name");

    parser_expect(parser, TOK_COLON, "Expected ':' after field name");
    parser_expect(parser, TOK_LBRACKET, "Expected '[' in bit range");

    uint32_t start = parser_expect_u32_literal(parser);

    parser_expect(parser, TOK_COLON, "Expected ':' in bit range");

    uint32_t end = parser_expect_u32_literal(parser);

    parser_expect(parser, TOK_RBRACKET, "Expected ']' after bit range");

    AccessKind access = parser_parse_access(parser);

    parser_match(parser, TOK_SEMICOLON);

    return ast_field_create(field_name, start, end, access);
}

static ASTRegister *parser_parse_register(Parser *parser) {
    parser_expect(parser, TOK_REGISTER, "Expected 'register'");

    const char *reg_name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected register name");

    parser_expect(parser, TOK_COLON, "Expected ':' after register name");

    Type *reg_type = (Type *)malloc(sizeof(Type));
    reg_type->kind = TYPE_U32; // default

    if (parser_check(parser, TOK_U8) || parser_check(parser, TOK_U16) ||
        parser_check(parser, TOK_U32) || parser_check(parser, TOK_U64) ||
        parser_check(parser, TOK_I8) || parser_check(parser, TOK_I16) ||
        parser_check(parser, TOK_I32) || parser_check(parser, TOK_I64) ||
        parser_check(parser, TOK_VOID)) {
        reg_type->kind = parser_parse_type_kind(parser);
    } else {
        // If no type token, keep default TYPE_U32.
    }

    parser_expect(parser, TOK_AT, "Expected '@' before offset");
    uint32_t offset = parser_expect_u32_literal(parser);

    parser_expect(parser, TOK_LBRACE, "Expected '{' after register");

    ASTRegister *reg = ast_register_create(reg_name, reg_type, offset);

    while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
        ASTField *field = parser_parse_field(parser);
        ast_register_add_field(reg, field);
    }

    parser_expect(parser, TOK_RBRACE, "Expected '}' after register");
    return reg;
}

static ASTPeripheral *parser_parse_peripheral(Parser *parser) {
    parser_expect(parser, TOK_PERIPHERAL, "Expected 'peripheral'");

    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected peripheral name");

    parser_expect(parser, TOK_AT, "Expected '@' before base address");
    uint32_t base = parser_expect_u32_literal(parser);

    parser_expect(parser, TOK_LBRACE, "Expected '{' after peripheral");

    ASTPeripheral *periph = ast_peripheral_create(name, base);

    while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
        ASTRegister *reg = parser_parse_register(parser);
        ast_peripheral_add_register(periph, reg);
    }

    parser_expect(parser, TOK_RBRACE, "Expected '}' after peripheral");
    return periph;
}

// ============================================================================
// Function parsing
// ============================================================================

// Existing brace-based fn syntax:
//   fn name() -> u32 { ... }
static ASTFunctionDef *parser_parse_fn(Parser *parser) {
    parser_expect(parser, TOK_FN, "Expected 'fn'");

    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected function name");

    parser_expect(parser, TOK_LPAREN, "Expected '('");
    parser_expect(parser, TOK_RPAREN, "Expected ')'");

    Type *return_type = (Type *)malloc(sizeof(Type));
    return_type->kind = TYPE_U32;

    if (parser_match(parser, TOK_ARROW)) {
        if (parser_check(parser, TOK_U8) || parser_check(parser, TOK_U16) ||
            parser_check(parser, TOK_U32) || parser_check(parser, TOK_U64) ||
            parser_check(parser, TOK_I8) || parser_check(parser, TOK_I16) ||
            parser_check(parser, TOK_I32) || parser_check(parser, TOK_I64) ||
            parser_check(parser, TOK_VOID)) {
            return_type->kind = parser_parse_type_kind(parser);
        } else {
            parser_error(parser, "Expected return type after '->'");
        }
    }

    parser_expect(parser, TOK_LBRACE, "Expected '{'");

    ASTStmt **statements = NULL;
    size_t stmt_count = 0;

    while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
        statements = (ASTStmt **)realloc(statements, (stmt_count + 1) * sizeof(ASTStmt *));
        statements[stmt_count++] = parser_parse_statement(parser);
    }

    parser_expect(parser, TOK_RBRACE, "Expected '}'");

    ASTStmt *body = ast_stmt_block(statements, stmt_count);
    return ast_function_def(name, return_type, NULL, NULL, 0, body);
}

// Nim-style proc/func syntax:
//   proc name(): u32 = return 0xFF
//   func name(): u32 = { return 0xFF; }
static ASTFunctionDef *parser_parse_proc_func(Parser *parser) {
    // consume proc/func
    if (!(parser_match(parser, TOK_PROC) || parser_match(parser, TOK_FUNC))) {
        parser_error(parser, "Expected 'proc' or 'func'");
        return NULL;
    }

    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected function name");

    parser_expect(parser, TOK_LPAREN, "Expected '('");
    parser_expect(parser, TOK_RPAREN, "Expected ')'");

    parser_expect(parser, TOK_COLON, "Expected ':' before return type");

    Type *return_type = (Type *)malloc(sizeof(Type));
    return_type->kind = parser_parse_type_kind(parser);

    // Your lexer emits TOK_ASSIGN for '=' in basic.bitn, so accept TOK_ASSIGN
    // and also TOK_EQUAL if you later switch to that.
    if (!(parser_match(parser, TOK_ASSIGN) || parser_match(parser, TOK_EQUAL))) {
        parser_error(parser, "Expected '=' before function body");
        // create an empty body so callers can continue
        return ast_function_def(name, return_type, NULL, NULL, 0, ast_stmt_block(NULL, 0));
    }

    ASTStmt *body = NULL;

    // Option A: brace block after '='
    if (parser_match(parser, TOK_LBRACE)) {
        ASTStmt **statements = NULL;
        size_t stmt_count = 0;

        while (!parser_check(parser, TOK_RBRACE) && !parser_check(parser, TOK_EOF)) {
            statements = (ASTStmt **)realloc(statements, (stmt_count + 1) * sizeof(ASTStmt *));
            statements[stmt_count++] = parser_parse_statement(parser);
        }

        parser_expect(parser, TOK_RBRACE, "Expected '}'");
        body = ast_stmt_block(statements, stmt_count);
    }
    // Option B: single statement after '=' (works for your examples/basic.bitn)
    else {
        ASTStmt **statements = (ASTStmt **)malloc(sizeof(ASTStmt *));
        statements[0] = parser_parse_statement(parser);
        body = ast_stmt_block(statements, 1);
    }

    return ast_function_def(name, return_type, NULL, NULL, 0, body);
}

// ============================================================================
// Program parsing
// ============================================================================

ASTProgram *parser_parse_program(Parser *parser) {
    ASTProgram *program = ast_program_create();

    while (!parser_check(parser, TOK_EOF) && !parser_has_error(parser)) {
        if (parser_check(parser, TOK_FN)) {
            ASTFunctionDef *func = parser_parse_fn(parser);
            if (func) ast_program_add_function(program, func);
            continue;
        }

        if (parser_check(parser, TOK_PROC) || parser_check(parser, TOK_FUNC)) {
            ASTFunctionDef *func = parser_parse_proc_func(parser);
            if (func) ast_program_add_function(program, func);
            continue;
        }

        if (parser_check(parser, TOK_PERIPHERAL)) {
            ASTPeripheral *p = parser_parse_peripheral(parser);
            if (p) ast_program_add_peripheral(program, p);
            continue;
        }

        parser_error(parser, "Expected 'fn', 'proc', 'func', or 'peripheral'");

        // Recovery: skip until something that can start a top-level item.
        while (!parser_check(parser, TOK_FN) &&
               !parser_check(parser, TOK_PROC) &&
               !parser_check(parser, TOK_FUNC) &&
               !parser_check(parser, TOK_PERIPHERAL) &&
               !parser_check(parser, TOK_EOF)) {
            parser_advance(parser);
        }
    }

    return program;
}
