#include "../include/lexer.h"
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

static void lexer_advance(Lexer *lexer) {
    if (lexer->current == '\n') {
        lexer->line++;
        lexer->column = 0;
    }

    if (lexer->pos < strlen(lexer->input)) {
        lexer->current = lexer->input[lexer->pos++];
        lexer->column++;
    } else {
        lexer->current = '\0';
    }
}

static char lexer_peek(Lexer *lexer) {
    if (lexer->pos < strlen(lexer->input)) {
        return lexer->input[lexer->pos];
    }
    return '\0';
}

Lexer *lexer_create(const char *input) {
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->pos = 1;  // ← FIX: Start at 1, not 0
    lexer->line = 1;
    lexer->column = 0;
    lexer->current = input[0];
    return lexer;
}

void lexer_free(Lexer *lexer) {
    free(lexer);
}

void lexer_skip_whitespace(Lexer *lexer) {
    while (isspace(lexer->current)) {
        lexer_advance(lexer);
    }
}

void lexer_skip_comment(Lexer *lexer) {
    if (lexer->current == '/' && lexer_peek(lexer) == '/') {
        while (lexer->current != '\n' && lexer->current != '\0') {
            lexer_advance(lexer);
        }
    } else if (lexer->current == '/' && lexer_peek(lexer) == '*') {
        lexer_advance(lexer);
        lexer_advance(lexer);
        while (!(lexer->current == '*' && lexer_peek(lexer) == '/') && lexer->current != '\0') {
            lexer_advance(lexer);
        }
        if (lexer->current == '*') {
            lexer_advance(lexer);
            lexer_advance(lexer);
        }
    }
}

static Token lexer_make_token(TokenType type, const char *value, int length, int line, int column) {
    Token tok;
    tok.type = type;
    tok.value = value;
    tok.length = length;
    tok.line = line;
    tok.column = column;
    return tok;
}

static TokenType lexer_keyword_type(const char *word, int length) {
    if (length == 2 && strncmp(word, "fn", 2) == 0) return TOK_FN;
    if (length == 3 && strncmp(word, "let", 3) == 0) return TOK_LET;
    if (length == 3 && strncmp(word, "mut", 3) == 0) return TOK_MUT;
    if (length == 6 && strncmp(word, "return", 6) == 0) return TOK_RETURN;
    if (length == 2 && strncmp(word, "if", 2) == 0) return TOK_IF;
    if (length == 4 && strncmp(word, "else", 4) == 0) return TOK_ELSE;
    if (length == 5 && strncmp(word, "while", 5) == 0) return TOK_WHILE;
    if (length == 3 && strncmp(word, "for", 3) == 0) return TOK_FOR;
    if (length == 2 && strncmp(word, "in", 2) == 0) return TOK_IN;
    if (length == 4 && strncmp(word, "true", 4) == 0) return TOK_TRUE;
    if (length == 5 && strncmp(word, "false", 5) == 0) return TOK_FALSE;
    if (length == 2 && strncmp(word, "u8", 2) == 0) return TOK_U8;
    if (length == 3 && strncmp(word, "u16", 3) == 0) return TOK_U16;
    if (length == 3 && strncmp(word, "u32", 3) == 0) return TOK_U32;
    if (length == 3 && strncmp(word, "u64", 3) == 0) return TOK_U64;
    if (length == 2 && strncmp(word, "i8", 2) == 0) return TOK_I8;
    if (length == 3 && strncmp(word, "i16", 3) == 0) return TOK_I16;
    if (length == 3 && strncmp(word, "i32", 3) == 0) return TOK_I32;
    if (length == 3 && strncmp(word, "i64", 3) == 0) return TOK_I64;
    if (length == 4 && strncmp(word, "void", 4) == 0) return TOK_VOID;
    return TOK_IDENTIFIER;
}

Token lexer_next_token(Lexer *lexer) {
    // Skip whitespace and comments
    while (1) {
        lexer_skip_whitespace(lexer);
        if (lexer->current == '/' && (lexer_peek(lexer) == '/' || lexer_peek(lexer) == '*')) {
            lexer_skip_comment(lexer);
        } else {
            break;
        }
    }

    int line = (int)lexer->line;
    int column = (int)lexer->column;

    // Parse numbers (decimal, hex, binary)
    if (isdigit(lexer->current)) {
        const char *start = &lexer->input[lexer->pos - 1];
        int length = 0;

        // Check for hex (0x) or binary (0b)
        if (lexer->current == '0' && (lexer_peek(lexer) == 'x' || lexer_peek(lexer) == 'X' ||
                                      lexer_peek(lexer) == 'b' || lexer_peek(lexer) == 'B')) {
            length++;
            lexer_advance(lexer); // skip '0'
            length++;
            lexer_advance(lexer); // skip 'x' or 'b'
            
            // Parse hex or binary digits
            while (isxdigit(lexer->current) || lexer->current == 'b' || lexer->current == 'B') {
                length++;
                lexer_advance(lexer);
            }
        } else {
            // Parse decimal
            while (isdigit(lexer->current)) {
                length++;
                lexer_advance(lexer);
            }
        }
        return lexer_make_token(TOK_NUMBER, start, length, line, column);
    }

    // Parse identifiers and keywords
    if (isalpha(lexer->current) || lexer->current == '_') {
        const char *start = &lexer->input[lexer->pos - 1];
        int length = 0;
        while (isalnum(lexer->current) || lexer->current == '_') {
            length++;
            lexer_advance(lexer);
        }
        TokenType type = lexer_keyword_type(start, length);
        return lexer_make_token(type, start, length, line, column);
    }

    // Parse operators and delimiters
    const char *current_char = &lexer->input[lexer->pos - 1];

    if (lexer->current == '+') { lexer_advance(lexer); return lexer_make_token(TOK_PLUS, current_char, 1, line, column); }
    if (lexer->current == '-') {
        lexer_advance(lexer);
        if (lexer->current == '>') {
            lexer_advance(lexer);
            return lexer_make_token(TOK_ARROW, current_char, 2, line, column);
        }
        return lexer_make_token(TOK_MINUS, current_char, 1, line, column);
    }
    if (lexer->current == '*') { lexer_advance(lexer); return lexer_make_token(TOK_STAR, current_char, 1, line, column); }
    if (lexer->current == '/') { lexer_advance(lexer); return lexer_make_token(TOK_SLASH, current_char, 1, line, column); }
    if (lexer->current == '%') { lexer_advance(lexer); return lexer_make_token(TOK_PERCENT, current_char, 1, line, column); }
    if (lexer->current == '&') { lexer_advance(lexer); return lexer_make_token(TOK_AND, current_char, 1, line, column); }
    if (lexer->current == '|') { lexer_advance(lexer); return lexer_make_token(TOK_OR, current_char, 1, line, column); }
    if (lexer->current == '^') { lexer_advance(lexer); return lexer_make_token(TOK_XOR, current_char, 1, line, column); }
    if (lexer->current == '~') { lexer_advance(lexer); return lexer_make_token(TOK_NOT, current_char, 1, line, column); }

    if (lexer->current == '<') {
        lexer_advance(lexer);
        if (lexer->current == '<') {
            lexer_advance(lexer);
            return lexer_make_token(TOK_LSHIFT, current_char, 2, line, column);
        }
        if (lexer->current == '=') {
            lexer_advance(lexer);
            return lexer_make_token(TOK_LE, current_char, 2, line, column);
        }
        return lexer_make_token(TOK_LT, current_char, 1, line, column);
    }

    if (lexer->current == '>') {
        lexer_advance(lexer);
        if (lexer->current == '>') {
            lexer_advance(lexer);
            return lexer_make_token(TOK_RSHIFT, current_char, 2, line, column);
        }
        if (lexer->current == '=') {
            lexer_advance(lexer);
            return lexer_make_token(TOK_GE, current_char, 2, line, column);
        }
        return lexer_make_token(TOK_GT, current_char, 1, line, column);
    }

    if (lexer->current == '=') {
        lexer_advance(lexer);
        if (lexer->current == '=') {
            lexer_advance(lexer);
            return lexer_make_token(TOK_EQ, current_char, 2, line, column);
        }
        return lexer_make_token(TOK_ASSIGN, current_char, 1, line, column);
    }

    if (lexer->current == '!') {
        lexer_advance(lexer);
        if (lexer->current == '=') {
            lexer_advance(lexer);
            return lexer_make_token(TOK_NE, current_char, 2, line, column);
        }
        return lexer_make_token(TOK_NOT, current_char, 1, line, column);
    }

    // Single character tokens
    switch (lexer->current) {
        case '(': lexer_advance(lexer); return lexer_make_token(TOK_LPAREN, current_char, 1, line, column);
        case ')': lexer_advance(lexer); return lexer_make_token(TOK_RPAREN, current_char, 1, line, column);
        case '{': lexer_advance(lexer); return lexer_make_token(TOK_LBRACE, current_char, 1, line, column);
        case '}': lexer_advance(lexer); return lexer_make_token(TOK_RBRACE, current_char, 1, line, column);
        case '[': lexer_advance(lexer); return lexer_make_token(TOK_LBRACKET, current_char, 1, line, column);
        case ']': lexer_advance(lexer); return lexer_make_token(TOK_RBRACKET, current_char, 1, line, column);
        case ':': lexer_advance(lexer); return lexer_make_token(TOK_COLON, current_char, 1, line, column);
        case ';': lexer_advance(lexer); return lexer_make_token(TOK_SEMICOLON, current_char, 1, line, column);
        case ',': lexer_advance(lexer); return lexer_make_token(TOK_COMMA, current_char, 1, line, column);
        case '.': lexer_advance(lexer); return lexer_make_token(TOK_DOT, current_char, 1, line, column);
        case '\0': return lexer_make_token(TOK_EOF, "", 0, line, column);
        default: lexer_advance(lexer); return lexer_make_token(TOK_ERROR, current_char, 1, line, column);
    }
}
