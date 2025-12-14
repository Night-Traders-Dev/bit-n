#include "token.h"
#include <string.h>

const char *token_type_name(TokenType type) {
    switch (type) {
        // Literals
        case TOK_NUMBER: return "NUMBER";
        case TOK_IDENTIFIER: return "IDENTIFIER";
        case TOK_STRING: return "STRING";
        
        // Keywords - Control flow
        case TOK_IF: return "IF";
        case TOK_ELSE: return "ELSE";
        case TOK_WHILE: return "WHILE";
        case TOK_FOR: return "FOR";
        case TOK_IN: return "IN";
        case TOK_RETURN: return "RETURN";
        
        // Keywords - Function definitions (NEW)
        case TOK_PROC: return "PROC";
        case TOK_FUNC: return "FUNC";
        
        // Keywords - Variable declaration
        case TOK_LET: return "LET";
        case TOK_VAR: return "VAR";
        
        // Keywords - Boolean
        case TOK_TRUE: return "TRUE";
        case TOK_FALSE: return "FALSE";
        
        // Keywords - Types
        case TOK_U8: return "U8";
        case TOK_U16: return "U16";
        case TOK_U32: return "U32";
        case TOK_U64: return "U64";
        case TOK_I8: return "I8";
        case TOK_I16: return "I16";
        case TOK_I32: return "I32";
        case TOK_I64: return "I64";
        case TOK_VOID: return "VOID";
        
        // Operators - Logical/Bitwise
        case TOK_AND: return "AND";
        case TOK_OR: return "OR";
        case TOK_XOR: return "XOR";
        case TOK_NOT: return "NOT";
        case TOK_LSHIFT: return "LSHIFT";
        case TOK_RSHIFT: return "RSHIFT";
        case TOK_LROTATE: return "LROTATE";
        case TOK_RROTATE: return "RROTATE";
        
        // Operators - Arithmetic
        case TOK_PLUS: return "PLUS";
        case TOK_MINUS: return "MINUS";
        case TOK_STAR: return "STAR";
        case TOK_SLASH: return "SLASH";
        case TOK_PERCENT: return "PERCENT";
        
        // Operators - Comparison
        case TOK_EQ: return "EQ";
        case TOK_NE: return "NE";
        case TOK_LT: return "LT";
        case TOK_GT: return "GT";
        case TOK_LE: return "LE";
        case TOK_GE: return "GE";
        
        // Assignment
        case TOK_ASSIGN: return "ASSIGN";
        case TOK_PLUS_ASSIGN: return "PLUS_ASSIGN";
        case TOK_MINUS_ASSIGN: return "MINUS_ASSIGN";
        case TOK_AND_ASSIGN: return "AND_ASSIGN";
        case TOK_OR_ASSIGN: return "OR_ASSIGN";
        case TOK_XOR_ASSIGN: return "XOR_ASSIGN";
        
        // Delimiters
        case TOK_LPAREN: return "LPAREN";
        case TOK_RPAREN: return "RPAREN";
        case TOK_LBRACKET: return "LBRACKET";
        case TOK_RBRACKET: return "RBRACKET";
        case TOK_COLON: return "COLON";
        case TOK_SEMICOLON: return "SEMICOLON";
        case TOK_COMMA: return "COMMA";
        case TOK_DOT: return "DOT";
        
        // NEW: NIM-STYLE SYNTAX TOKENS
        case TOK_EQUAL: return "EQUAL";
        case TOK_INDENT: return "INDENT";
        case TOK_DEDENT: return "DEDENT";
        
        // DEPRECATED
        case TOK_FN: return "FN";
        case TOK_MUT: return "MUT";
        case TOK_ARROW: return "ARROW";
        case TOK_LBRACE: return "LBRACE";
        case TOK_RBRACE: return "RBRACE";
        case TOK_DARROW: return "DARROW";
        
        // Special
        case TOK_EOF: return "EOF";
        case TOK_ERROR: return "ERROR";
        
        default: return "UNKNOWN";
    }
}

void token_print(Token *tok) {
    printf("Token(%s, line=%d, col=%d, len=%d)\n",
           token_type_name(tok->type),
           tok->line,
           tok->column,
           tok->length);
}
