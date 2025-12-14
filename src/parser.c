#include "../include/parser.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

Parser *parser_create(const char *input) {
    Parser *parser = (Parser *)malloc(sizeof(Parser));
    parser->lexer = lexer_create(input);
    parser->current = lexer_next_token(parser->lexer);
    parser->peek = lexer_next_token(parser->lexer);
    parser->error = 0;
    return parser;
}

void parser_free(Parser *parser) {
    lexer_free(parser->lexer);
    free(parser);
}

void parser_error(Parser *parser, const char *message) {
    fprintf(stderr, "Parser error at %d:%d: %s\n", 
            parser->current.line, parser->current.column, message);
    parser->error = 1;
}

int parser_has_error(Parser *parser) { return parser->error; }

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

static ASTExpr *parser_parse_expression(Parser *parser);
static ASTStmt *parser_parse_statement(Parser *parser);

static ASTExpr *parser_parse_primary(Parser *parser) {
    switch (parser->current.type) {
        case TOK_NUMBER: {
            uint64_t value = 0;
            sscanf(parser->current.value, "%lu", &value);
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
    if (parser_match(parser, TOK_NOT)) {
        return ast_expr_unary_op(UOP_NOT, parser_parse_unary(parser));
    }
    if (parser_match(parser, TOK_MINUS)) {
        return ast_expr_unary_op(UOP_NEG, parser_parse_unary(parser));
    }
    return parser_parse_primary(parser);
}

static ASTExpr *parser_parse_multiplicative(Parser *parser) {
    ASTExpr *expr = parser_parse_unary(parser);
    while (parser->current.type == TOK_STAR || parser->current.type == TOK_SLASH) {
        BinaryOp op = parser_match(parser, TOK_STAR) ? BOP_MUL : BOP_DIV;
        if (op == BOP_DIV) parser_advance(parser);
        ASTExpr *right = parser_parse_unary(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }
    return expr;
}

static ASTExpr *parser_parse_additive(Parser *parser) {
    ASTExpr *expr = parser_parse_multiplicative(parser);
    while (parser->current.type == TOK_PLUS || parser->current.type == TOK_MINUS) {
        BinaryOp op = parser_match(parser, TOK_PLUS) ? BOP_ADD : BOP_SUB;
        if (op == BOP_SUB) parser_advance(parser);
        ASTExpr *right = parser_parse_multiplicative(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }
    return expr;
}

static ASTExpr *parser_parse_shift(Parser *parser) {
    ASTExpr *expr = parser_parse_additive(parser);
    while (parser->current.type == TOK_LSHIFT || parser->current.type == TOK_RSHIFT) {
        BinaryOp op = parser_match(parser, TOK_LSHIFT) ? BOP_LSHIFT : BOP_RSHIFT;
        if (op == BOP_RSHIFT) parser_advance(parser);
        ASTExpr *right = parser_parse_additive(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }
    return expr;
}

static ASTExpr *parser_parse_comparison(Parser *parser) {
    ASTExpr *expr = parser_parse_shift(parser);
    while (parser->current.type == TOK_LT || parser->current.type == TOK_GT || 
           parser->current.type == TOK_LE || parser->current.type == TOK_GE) {
        BinaryOp op;
        if (parser_match(parser, TOK_LT)) op = BOP_LT;
        else if (parser_match(parser, TOK_GT)) op = BOP_GT;
        else if (parser_match(parser, TOK_LE)) op = BOP_LE;
        else { parser_match(parser, TOK_GE); op = BOP_GE; }
        ASTExpr *right = parser_parse_shift(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }
    return expr;
}

static ASTExpr *parser_parse_equality(Parser *parser) {
    ASTExpr *expr = parser_parse_comparison(parser);
    while (parser->current.type == TOK_EQ || parser->current.type == TOK_NE) {
        BinaryOp op = parser_match(parser, TOK_EQ) ? BOP_EQ : BOP_NE;
        if (op == BOP_NE) parser_advance(parser);
        ASTExpr *right = parser_parse_comparison(parser);
        expr = ast_expr_binary_op(op, expr, right);
    }
    return expr;
}

static ASTExpr *parser_parse_and_expr(Parser *parser) {
    ASTExpr *expr = parser_parse_equality(parser);
    while (parser_match(parser, TOK_AND)) {
        ASTExpr *right = parser_parse_equality(parser);
        expr = ast_expr_binary_op(BOP_AND, expr, right);
    }
    return expr;
}

static ASTExpr *parser_parse_or_expr(Parser *parser) {
    ASTExpr *expr = parser_parse_and_expr(parser);
    while (parser_match(parser, TOK_OR)) {
        ASTExpr *right = parser_parse_and_expr(parser);
        expr = ast_expr_binary_op(BOP_OR, expr, right);
    }
    return expr;
}

static ASTExpr *parser_parse_expression(Parser *parser) {
    return parser_parse_or_expr(parser);
}

static ASTStmt *parser_parse_statement(Parser *parser) {
    if (parser_match(parser, TOK_RETURN)) {
        ASTExpr *value = NULL;
        if (!parser_check(parser, TOK_SEMICOLON) && !parser_check(parser, TOK_RBRACE)) {
            value = parser_parse_expression(parser);
        }
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

static ASTFunctionDef *parser_parse_function(Parser *parser) {
    parser_expect(parser, TOK_FN, "Expected 'fn'");
    const char *name = parser->current.value;
    parser_expect(parser, TOK_IDENTIFIER, "Expected function name");
    parser_expect(parser, TOK_LPAREN, "Expected '('");
    parser_expect(parser, TOK_RPAREN, "Expected ')'");
    
    Type *return_type = (Type *)malloc(sizeof(Type));
    return_type->kind = TYPE_U32;
    if (parser_match(parser, TOK_ARROW)) {
        if (parser_match(parser, TOK_U32)) return_type->kind = TYPE_U32;
        else if (parser_match(parser, TOK_VOID)) return_type->kind = TYPE_VOID;
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

ASTProgram *parser_parse_program(Parser *parser) {
    ASTProgram *program = ast_program_create();
    
    while (!parser_check(parser, TOK_EOF)) {
        if (parser_check(parser, TOK_FN)) {
            ASTFunctionDef *func = parser_parse_function(parser);
            ast_program_add_function(program, func);
        } else {
            parser_error(parser, "Expected function definition");
            while (!parser_check(parser, TOK_FN) && !parser_check(parser, TOK_EOF)) {
                parser_advance(parser);
            }
            if (parser_check(parser, TOK_EOF)) {
                break;
            }
        }
    }
    
    return program;
}
