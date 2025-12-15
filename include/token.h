#ifndef BITN_TOKEN_H
#define BITN_TOKEN_H

#include <stdint.h>
#include <stddef.h>

typedef enum {
    // Literals
    TOK_NUMBER, TOK_IDENTIFIER, TOK_STRING,

    // Keywords - Control flow
    TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR, TOK_IN, TOK_RETURN,

    // Keywords - Function definitions (Nim-style)
    TOK_PROC, TOK_FUNC,

    // Keywords - Variable declarations
    TOK_LET, TOK_VAR,

    // Keywords - Boolean
    TOK_TRUE, TOK_FALSE,

    // Keywords - Types
    TOK_U8, TOK_U16, TOK_U32, TOK_U64, TOK_I8, TOK_I16, TOK_I32, TOK_I64, TOK_VOID,

    // Keywords - DSL Peripherals
    TOK_PERIPHERAL, TOK_REGISTER, TOK_FIELD,
    TOK_RO, TOK_WO, TOK_RW, TOK_W1C,

    // Operators - Logical/Bitwise
    TOK_AND, TOK_OR, TOK_XOR, TOK_NOT, TOK_LSHIFT, TOK_RSHIFT, TOK_LROTATE, TOK_RROTATE,

    // Operators - Arithmetic
    TOK_PLUS, TOK_MINUS, TOK_STAR, TOK_SLASH, TOK_PERCENT,

    // Operators - Comparison
    TOK_EQ, TOK_NE, TOK_LT, TOK_GT, TOK_LE, TOK_GE,

    // Assignment
    TOK_ASSIGN, TOK_PLUS_ASSIGN, TOK_MINUS_ASSIGN, TOK_AND_ASSIGN, TOK_OR_ASSIGN, TOK_XOR_ASSIGN,

    // Delimiters
    TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACKET, TOK_RBRACKET,
    TOK_COLON, TOK_SEMICOLON, TOK_COMMA, TOK_DOT,
    TOK_AT,          // '@' for peripheral base/offset

    // Nim-style block tokens
    TOK_EQUAL,       // '=' for block start (distinct from assignment)
    TOK_INDENT,      // Synthetic token for indentation increase
    TOK_DEDENT,      // Synthetic token for indentation decrease

    // Deprecated (kept for compatibility)
    TOK_FN, TOK_MUT, TOK_ARROW, TOK_LBRACE, TOK_RBRACE, TOK_DARROW,

    // Special
    TOK_EOF, TOK_ERROR,
} TokenType;

typedef struct {
    TokenType type;
    const char *value;
    int line;
    int column;
    int length;
} Token;

const char *token_type_name(TokenType type);
void token_print(Token *tok);

#endif // BITN_TOKEN_H
