#include "parser.h"
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
    fprintf(stderr, "Parse Error at line %d, col %d: %s\n",
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
    return parser->current.type == type;
}

static int parser_expect(Parser *parser, TokenType type, const char *message) {
    if (!parser_match(parser, type)) {
        parser_error(parser, message);
        return 0;
    }
    parser_advance(parser);
    return 1;
}

static int parser_consume(Parser *parser, TokenType type) {
    if (parser_match(parser, type)) {
        parser_advance(parser);
        return 1;
    }
    return 0;
}

// ============================================================================
// OPERATOR NAME RECOGNITION (NEW FOR NIM-STYLE OPERATORS)
// ============================================================================

// ✅ Map operator names to binary operators
static BinaryOp get_binary_operator_for_name(const char *name, int length) {
    if (length == 3 && strncmp(name, "add", 3) == 0) return BOP_ADD;
    if (length == 3 && strncmp(name, "sub", 3) == 0) return BOP_SUB;
    if (length == 3 && strncmp(name, "mul", 3) == 0) return BOP_MUL;
    if (length == 3 && strncmp(name, "div", 3) == 0) return BOP_DIV;
    if (length == 3 && strncmp(name, "mod", 3) == 0) return BOP_MOD;
    if (length == 6 && strncmp(name, "bitand", 6) == 0) return BOP_AND;
    if (length == 5 && strncmp(name, "bitor", 5) == 0) return BOP_OR;
    if (length == 6 && strncmp(name, "bitxor", 6) == 0) return BOP_XOR;
    if (length == 3 && strncmp(name, "shl", 3) == 0) return BOP_LSHIFT;
    if (length == 3 && strncmp(name, "shr", 3) == 0) return BOP_RSHIFT;
    if (length == 2 && strncmp(name, "eq", 2) == 0) return BOP_EQ;
    if (length == 2 && strncmp(name, "ne", 2) == 0) return BOP_NE;
    if (length == 2 && strncmp(name, "lt", 2) == 0) return BOP_LT;
    if (length == 2 && strncmp(name, "le", 2) == 0) return BOP_LE;
    if (length == 2 && strncmp(name, "gt", 2) == 0) return BOP_GT;
    if (length == 2 && strncmp(name, "ge", 2) == 0) return BOP_GE;
    return -1;  // Not a binary operator name
}

// ✅ Map operator names to unary operators
static UnaryOp get_unary_operator_for_name(const char *name, int length) {
    if (length == 3 && strncmp(name, "neg", 3) == 0) return UOP_NEG;
    if (length == 3 && strncmp(name, "not", 3) == 0) return UOP_NOT;
    if (length == 6 && strncmp(name, "bitnot", 6) == 0) return UOP_BIT_NOT;
    return -1;  // Not a unary operator name
}

// ============================================================================
// TYPE PARSING
// ============================================================================

Type *parse_type(Parser *parser) {
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
    
    parser_advance(parser);
    
    Type *type = (Type *)malloc(sizeof(Type));
    type->kind = kind;
    return type;
}

// ============================================================================
// EXPRESSION PARSING
// ============================================================================

// Forward declarations
ASTExpr *parse_expression(Parser *parser);
ASTExpr *parse_primary(Parser *parser);
ASTExpr *parse_unary(Parser *parser);
ASTExpr *parse_multiplicative(Parser *parser);
ASTExpr *parse_additive(Parser *parser);
ASTExpr *parse_shift(Parser *parser);
ASTExpr *parse_comparison(Parser *parser);
ASTExpr *parse_equality(Parser *parser);
ASTExpr *parse_bitwise_and(Parser *parser);
ASTExpr *parse_bitwise_xor(Parser *parser);
ASTExpr *parse_bitwise_or(Parser *parser);

ASTExpr *parse_primary(Parser *parser) {
    // Numbers
    if (parser_match(parser, TOK_NUMBER)) {
        uint64_t value = strtoull(parser->current.value, NULL, 0);
        parser_advance(parser);
        return ast_expr_number(value, TYPE_U32);
    }
    
    // Identifiers (variables, function calls, or operator names)
    if (parser_match(parser, TOK_IDENTIFIER)) {
        const char *name = parser->current.value;
        int name_len = parser->current.length;
        parser_advance(parser);
        
        // Check if it's a unary operator name: neg(x), not(x), bitnot(x)
        if (parser_match(parser, TOK_LPAREN)) {
            UnaryOp unary_op = get_unary_operator_for_name(name, name_len);
            if (unary_op >= 0) {
                // It's a unary operator function call
                parser_advance(parser);  // consume '('
                ASTExpr *arg = parse_expression(parser);
                parser_expect(parser, TOK_RPAREN, "Expected ')'");
                return ast_expr_unary_op(unary_op, arg);
            }
            
            // It's a regular function call
            parser_advance(parser);  // consume '('
            
            // Parse arguments
            ASTExpr **args = NULL;
            size_t arg_count = 0;
            size_t arg_capacity = 10;
            args = malloc(arg_capacity * sizeof(ASTExpr*));
            
            if (!parser_match(parser, TOK_RPAREN)) {
                while (1) {
                    if (arg_count >= arg_capacity) {
                        arg_capacity *= 2;
                        args = realloc(args, arg_capacity * sizeof(ASTExpr*));
                    }
                    args[arg_count++] = parse_expression(parser);
                    
                    if (!parser_consume(parser, TOK_COMMA)) {
                        break;
                    }
                }
            }
            
            parser_expect(parser, TOK_RPAREN, "Expected ')' after arguments");
            
            // Check if it's a binary operator function: add(x, y), bitand(x, y), etc
            if (arg_count == 2) {
                BinaryOp binop = get_binary_operator_for_name(name, name_len);
                if (binop >= 0) {
                    // Create binary operator AST node
                    ASTExpr *result = ast_expr_binary_op(binop, args[0], args[1]);
                    free(args);
                    return result;
                }
            }
            
            // Regular function call
            ASTExpr *func = ast_expr_identifier(name);
            return ast_expr_call(func, args, arg_count);
        }
        
        // Simple identifier (variable reference)
        return ast_expr_identifier(name);
    }
    
    // Boolean literals
    if (parser_match(parser, TOK_TRUE)) {
        parser_advance(parser);
        return ast_expr_true();
    }
    if (parser_match(parser, TOK_FALSE)) {
        parser_advance(parser);
        return ast_expr_false();
    }
    
    // Parenthesized expression
    if (parser_match(parser, TOK_LPAREN)) {
        parser_advance(parser);
        ASTExpr *expr = parse_expression(parser);
        parser_expect(parser, TOK_RPAREN, "Expected ')' after expression");
        return expr;
    }
    
    parser_error(parser, "Expected primary expression");
    return NULL;
}

ASTExpr *parse_unary(Parser *parser) {
    // Unary operators: -, ~, !
    if (parser_match(parser, TOK_MINUS)) {
        parser_advance(parser);
        ASTExpr *operand = parse_unary(parser);
        return ast_expr_unary_op(UOP_NEG, operand);
    }
    if (parser_match(parser, TOK_NOT)) {
        parser_advance(parser);
        ASTExpr *operand = parse_unary(parser);
        return ast_expr_unary_op(UOP_NOT, operand);
    }
    if (parser_match(parser, TOK_XOR)) {
        parser_advance(parser);
        ASTExpr *operand = parse_unary(parser);
        return ast_expr_unary_op(UOP_BIT_NOT, operand);
    }
    
    return parse_primary(parser);
}

ASTExpr *parse_multiplicative(Parser *parser) {
    ASTExpr *left = parse_unary(parser);
    
    while (parser_match(parser, TOK_STAR) || parser_match(parser, TOK_SLASH) || parser_match(parser, TOK_PERCENT)) {
        BinaryOp op;
        if (parser_match(parser, TOK_STAR)) {
            op = BOP_MUL;
        } else if (parser_match(parser, TOK_SLASH)) {
            op = BOP_DIV;
        } else {
            op = BOP_MOD;
        }
        parser_advance(parser);
        ASTExpr *right = parse_unary(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    
    return left;
}

ASTExpr *parse_additive(Parser *parser) {
    ASTExpr *left = parse_multiplicative(parser);
    
    while (parser_match(parser, TOK_PLUS) || parser_match(parser, TOK_MINUS)) {
        BinaryOp op = parser_match(parser, TOK_PLUS) ? BOP_ADD : BOP_SUB;
        parser_advance(parser);
        ASTExpr *right = parse_multiplicative(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    
    return left;
}

ASTExpr *parse_shift(Parser *parser) {
    ASTExpr *left = parse_additive(parser);
    
    while (parser_match(parser, TOK_LSHIFT) || parser_match(parser, TOK_RSHIFT)) {
        BinaryOp op = parser_match(parser, TOK_LSHIFT) ? BOP_LSHIFT : BOP_RSHIFT;
        parser_advance(parser);
        ASTExpr *right = parse_additive(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    
    return left;
}

ASTExpr *parse_comparison(Parser *parser) {
    ASTExpr *left = parse_shift(parser);
    
    while (parser_match(parser, TOK_LT) || parser_match(parser, TOK_LE) ||
           parser_match(parser, TOK_GT) || parser_match(parser, TOK_GE)) {
        BinaryOp op;
        if (parser_match(parser, TOK_LT)) op = BOP_LT;
        else if (parser_match(parser, TOK_LE)) op = BOP_LE;
        else if (parser_match(parser, TOK_GT)) op = BOP_GT;
        else op = BOP_GE;
        
        parser_advance(parser);
        ASTExpr *right = parse_shift(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    
    return left;
}

ASTExpr *parse_equality(Parser *parser) {
    ASTExpr *left = parse_comparison(parser);
    
    while (parser_match(parser, TOK_EQ) || parser_match(parser, TOK_NE)) {
        BinaryOp op = parser_match(parser, TOK_EQ) ? BOP_EQ : BOP_NE;
        parser_advance(parser);
        ASTExpr *right = parse_comparison(parser);
        left = ast_expr_binary_op(op, left, right);
    }
    
    return left;
}

ASTExpr *parse_bitwise_and(Parser *parser) {
    ASTExpr *left = parse_equality(parser);
    
    while (parser_match(parser, TOK_AND)) {
        parser_advance(parser);
        ASTExpr *right = parse_equality(parser);
        left = ast_expr_binary_op(BOP_AND, left, right);
    }
    
    return left;
}

ASTExpr *parse_bitwise_xor(Parser *parser) {
    ASTExpr *left = parse_bitwise_and(parser);
    
    while (parser_match(parser, TOK_XOR)) {
        parser_advance(parser);
        ASTExpr *right = parse_bitwise_and(parser);
        left = ast_expr_binary_op(BOP_XOR, left, right);
    }
    
    return left;
}

ASTExpr *parse_bitwise_or(Parser *parser) {
    ASTExpr *left = parse_bitwise_xor(parser);
    
    while (parser_match(parser, TOK_OR)) {
        parser_advance(parser);
        ASTExpr *right = parse_bitwise_xor(parser);
        left = ast_expr_binary_op(BOP_OR, left, right);
    }
    
    return left;
}

ASTExpr *parse_expression(Parser *parser) {
    return parse_bitwise_or(parser);
}

// ============================================================================
// STATEMENT PARSING
// ============================================================================

ASTStmt *parse_block(Parser *parser);
ASTStmt *parse_statement(Parser *parser);

// ✅ NEW: Parse indentation-based block
ASTStmt *parse_block(Parser *parser) {
    // Called after seeing INDENT token
    if (!parser_match(parser, TOK_INDENT)) {
        parser_error(parser, "Expected INDENT token");
        return NULL;
    }
    parser_advance(parser);  // consume INDENT
    
    ASTStmt **statements = NULL;
    size_t count = 0;
    size_t capacity = 10;
    statements = malloc(capacity * sizeof(ASTStmt*));
    
    // Parse statements until DEDENT
    while (!parser_match(parser, TOK_DEDENT) && !parser_match(parser, TOK_EOF) && !parser_has_error(parser)) {
        ASTStmt *stmt = parse_statement(parser);
        if (stmt) {
            if (count >= capacity) {
                capacity *= 2;
                statements = realloc(statements, capacity * sizeof(ASTStmt*));
            }
            statements[count++] = stmt;
        }
        
        // Skip optional semicolons
        while (parser_consume(parser, TOK_SEMICOLON)) {
            // Continue
        }
    }
    
    return ast_stmt_block(statements, count);
}

ASTStmt *parse_statement(Parser *parser) {
    // Variable declaration: let x: Type = init
    if (parser_match(parser, TOK_LET)) {
        parser_advance(parser);
        
        if (!parser_match(parser, TOK_IDENTIFIER)) {
            parser_error(parser, "Expected variable name");
            return NULL;
        }
        const char *name = parser->current.value;
        parser_advance(parser);
        
        parser_expect(parser, TOK_COLON, "Expected ':' after variable name");
        Type *type = parse_type(parser);
        
        ASTExpr *init = NULL;
        if (parser_consume(parser, TOK_ASSIGN)) {
            init = parse_expression(parser);
        }
        
        // Optional semicolon
        parser_consume(parser, TOK_SEMICOLON);
        
        return ast_stmt_var_decl(name, type, init, 0);  // 0 = immutable
    }
    
    // ✅ NEW: Mutable variable declaration: var x: Type = init
    if (parser_match(parser, TOK_VAR)) {
        parser_advance(parser);
        
        if (!parser_match(parser, TOK_IDENTIFIER)) {
            parser_error(parser, "Expected variable name");
            return NULL;
        }
        const char *name = parser->current.value;
        parser_advance(parser);
        
        parser_expect(parser, TOK_COLON, "Expected ':' after variable name");
        Type *type = parse_type(parser);
        
        ASTExpr *init = NULL;
        if (parser_consume(parser, TOK_ASSIGN)) {
            init = parse_expression(parser);
        }
        
        // Optional semicolon
        parser_consume(parser, TOK_SEMICOLON);
        
        return ast_stmt_var_decl(name, type, init, 1);  // 1 = mutable
    }
    
    // Return statement
    if (parser_match(parser, TOK_RETURN)) {
        parser_advance(parser);
        ASTExpr *value = NULL;
        if (!parser_match(parser, TOK_SEMICOLON) && !parser_match(parser, TOK_DEDENT) && !parser_match(parser, TOK_EOF)) {
            value = parse_expression(parser);
        }
        parser_consume(parser, TOK_SEMICOLON);
        return ast_stmt_return(value);
    }
    
    // If statement: if condition: block
    if (parser_match(parser, TOK_IF)) {
        parser_advance(parser);
        parse_expression(parser);  // condition (parsed but not used for now)
        parser_expect(parser, TOK_COLON, "Expected ':' after if condition");
        ASTStmt *body = parse_block(parser);
        
        if (parser_match(parser, TOK_DEDENT)) {
            parser_advance(parser);
        }
        
        // TODO: Handle else clause
        return body;  // Simplified
    }
    
    // While statement: while condition: block
    if (parser_match(parser, TOK_WHILE)) {
        parser_advance(parser);
        parse_expression(parser);  // condition (parsed but not used for now)
        parser_expect(parser, TOK_COLON, "Expected ':' after while condition");
        ASTStmt *body = parse_block(parser);
        
        if (parser_match(parser, TOK_DEDENT)) {
            parser_advance(parser);
        }
        
        return body;  // Simplified
    }
    
    // Expression statement
    ASTExpr *expr = parse_expression(parser);
    if (expr) {
        parser_consume(parser, TOK_SEMICOLON);
        return ast_stmt_expr(expr);
    }
    
    return NULL;
}

// ============================================================================
// FUNCTION PARSING (UPDATED FOR NIM-STYLE SYNTAX)
// ============================================================================

// ✅ NEW: Parse proc/func keywords
ASTFunctionDef *parse_function(Parser *parser) {
    // Expect proc or func
    if (!parser_match(parser, TOK_PROC) && !parser_match(parser, TOK_FUNC)) {
        parser_error(parser, "Expected 'proc' or 'func'");
        return NULL;
    }
    
    parser_advance(parser);  // consume proc/func
    
    // Get function name
    if (!parser_match(parser, TOK_IDENTIFIER)) {
        parser_error(parser, "Expected function name");
        return NULL;
    }
    const char *name = parser->current.value;
    parser_advance(parser);
    
    // Expect '('
    parser_expect(parser, TOK_LPAREN, "Expected '(' after function name");
    
    // Parse parameters
    char **param_names = NULL;
    Type **param_types = NULL;
    size_t param_count = 0;
    size_t param_capacity = 10;
    
    param_names = malloc(param_capacity * sizeof(char*));
    param_types = malloc(param_capacity * sizeof(Type*));
    
    if (!parser_match(parser, TOK_RPAREN)) {
        while (1) {
            if (!parser_match(parser, TOK_IDENTIFIER)) {
                parser_error(parser, "Expected parameter name");
                break;
            }
            const char *param_name = parser->current.value;
            parser_advance(parser);
            
            parser_expect(parser, TOK_COLON, "Expected ':' after parameter name");
            Type *param_type = parse_type(parser);
            
            if (param_count >= param_capacity) {
                param_capacity *= 2;
                param_names = realloc(param_names, param_capacity * sizeof(char*));
                param_types = realloc(param_types, param_capacity * sizeof(Type*));
            }
            param_names[param_count] = (char*)param_name;
            param_types[param_count] = param_type;
            param_count++;
            
            if (!parser_consume(parser, TOK_COMMA)) {
                break;
            }
        }
    }
    
    // Expect ')'
    parser_expect(parser, TOK_RPAREN, "Expected ')' after parameters");
    
    // Expect ':' (return type annotation start)
    parser_expect(parser, TOK_COLON, "Expected ':' before return type");
    
    // Parse return type
    Type *return_type = parse_type(parser);
    
    // Expect '=' (block start)
    parser_expect(parser, TOK_EQUAL, "Expected '=' before function body");
    
    // Parse body (indentation-based block)
    ASTStmt *body = parse_block(parser);
    
    // Expect DEDENT
    if (parser_match(parser, TOK_DEDENT)) {
        parser_advance(parser);
    }
    
    return ast_function_def(name, return_type, param_names, param_types, param_count, body);
}

// ============================================================================
// PROGRAM PARSING
// ============================================================================

ASTProgram *parser_parse_program(Parser *parser) {
    ASTProgram *program = ast_program_create();
    
    while (!parser_match(parser, TOK_EOF) && !parser_has_error(parser)) {
        // Skip any INDENT/DEDENT at top level
        if (parser_match(parser, TOK_INDENT) || parser_match(parser, TOK_DEDENT) ||
            parser_match(parser, TOK_SEMICOLON)) {
            parser_advance(parser);
            continue;
        }
        
        // Parse function
        if (parser_match(parser, TOK_PROC) || parser_match(parser, TOK_FUNC)) {
            ASTFunctionDef *func = parse_function(parser);
            if (func) {
                ast_program_add_function(program, func);
            }
        } else {
            parser_error(parser, "Expected function definition");
            break;
        }
    }
    
    return program;
}
