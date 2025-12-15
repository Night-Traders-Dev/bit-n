#include "../include/lexer.h"

#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void lexer_advance(Lexer *lexer) {
    if (lexer->current == '\n') {
        lexer->line++;
        lexer->column = 0;
    } else {
        lexer->column++;
    }

    if (lexer->input[lexer->pos] == '\0') {
        lexer->current = '\0';
        return;
    }

    lexer->pos++;
    lexer->current = lexer->input[lexer->pos];
}

static char lexer_peek(Lexer *lexer) {
    if (lexer->input[lexer->pos] == '\0') return '\0';
    return lexer->input[lexer->pos + 1];
}

Lexer *lexer_create(const char *input) {
    Lexer *lexer = (Lexer *)malloc(sizeof(Lexer));
    lexer->input = input;
    lexer->pos = 0;
    lexer->line = 1;
    lexer->column = 0;
    lexer->current = input[0];

    // Keep these initialized even if indentation tokens are not currently emitted.
    lexer->indent_depth = 0;
    lexer->pending_dedents = 0;
    lexer->indent_size = 0;
    lexer->last_line_indent = 0;
    lexer->at_line_start = 1;
    for (int i = 0; i < MAX_INDENT_DEPTH; i++) lexer->indent_stack[i] = 0;

    return lexer;
}

void lexer_free(Lexer *lexer) {
    free(lexer);
}

void lexer_skip_whitespace(Lexer *lexer) {
    while (lexer->current != '\0' && isspace((unsigned char)lexer->current)) {
        lexer_advance(lexer);
    }
}

void lexer_skip_comment(Lexer *lexer) {
    // Line comment: //
    if (lexer->current == '/' && lexer_peek(lexer) == '/') {
        while (lexer->current != '\n' && lexer->current != '\0') {
            lexer_advance(lexer);
        }
        return;
    }

    // Block comment: /* ... */
    if (lexer->current == '/' && lexer_peek(lexer) == '*') {
        lexer_advance(lexer); // '/'
        lexer_advance(lexer); // '*'
        while (lexer->current != '\0') {
            if (lexer->current == '*' && lexer_peek(lexer) == '/') {
                lexer_advance(lexer); // '*'
                lexer_advance(lexer); // '/'
                break;
            }
            lexer_advance(lexer);
        }
        return;
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
    // Deprecated/compat
    if (length == 2 && strncmp(word, "fn", 2) == 0) return TOK_FN;
    if (length == 3 && strncmp(word, "mut", 3) == 0) return TOK_MUT;

    // Nim-style
    if (length == 4 && strncmp(word, "proc", 4) == 0) return TOK_PROC;
    if (length == 4 && strncmp(word, "func", 4) == 0) return TOK_FUNC;
    if (length == 3 && strncmp(word, "var", 3) == 0) return TOK_VAR;

    // Control flow
    if (length == 3 && strncmp(word, "let", 3) == 0) return TOK_LET;
    if (length == 6 && strncmp(word, "return", 6) == 0) return TOK_RETURN;
    if (length == 2 && strncmp(word, "if", 2) == 0) return TOK_IF;
    if (length == 4 && strncmp(word, "else", 4) == 0) return TOK_ELSE;
    if (length == 5 && strncmp(word, "while", 5) == 0) return TOK_WHILE;
    if (length == 3 && strncmp(word, "for", 3) == 0) return TOK_FOR;
    if (length == 2 && strncmp(word, "in", 2) == 0) return TOK_IN;

    // Boolean
    if (length == 4 && strncmp(word, "true", 4) == 0) return TOK_TRUE;
    if (length == 5 && strncmp(word, "false", 5) == 0) return TOK_FALSE;

    // Types
    if (length == 2 && strncmp(word, "u8", 2) == 0) return TOK_U8;
    if (length == 3 && strncmp(word, "u16", 3) == 0) return TOK_U16;
    if (length == 3 && strncmp(word, "u32", 3) == 0) return TOK_U32;
    if (length == 3 && strncmp(word, "u64", 3) == 0) return TOK_U64;
    if (length == 2 && strncmp(word, "i8", 2) == 0) return TOK_I8;
    if (length == 3 && strncmp(word, "i16", 3) == 0) return TOK_I16;
    if (length == 3 && strncmp(word, "i32", 3) == 0) return TOK_I32;
    if (length == 3 && strncmp(word, "i64", 3) == 0) return TOK_I64;
    if (length == 4 && strncmp(word, "void", 4) == 0) return TOK_VOID;

    // DSL keywords
    if (length == 10 && strncmp(word, "peripheral", 10) == 0) return TOK_PERIPHERAL;
    if (length == 8 && strncmp(word, "register", 8) == 0) return TOK_REGISTER;
    if (length == 5 && strncmp(word, "field", 5) == 0) return TOK_FIELD;

    // DSL access specifiers
    if (length == 2 && strncmp(word, "ro", 2) == 0) return TOK_RO;
    if (length == 2 && strncmp(word, "wo", 2) == 0) return TOK_WO;
    if (length == 2 && strncmp(word, "rw", 2) == 0) return TOK_RW;
    if (length == 3 && strncmp(word, "w1c", 3) == 0) return TOK_W1C;

    return TOK_IDENTIFIER;
}

static Token lexer_read_string(Lexer *lexer) {
    int line = lexer->line;
    int column = lexer->column;

    // consume opening quote
    lexer_advance(lexer);

    size_t start_pos = lexer->pos;
    while (lexer->current != '"' && lexer->current != '\0') {
        // minimal escape handling: skip escaped quote
        if (lexer->current == '\\' && lexer_peek(lexer) != '\0') {
            lexer_advance(lexer);
        }
        lexer_advance(lexer);
    }

    size_t end_pos = lexer->pos;
    int length = (int)(end_pos - start_pos);

    // consume closing quote if present
    if (lexer->current == '"') lexer_advance(lexer);

    return lexer_make_token(TOK_STRING, &lexer->input[start_pos], length, line, column);
}

static Token lexer_read_number(Lexer *lexer) {
    int line = lexer->line;
    int column = lexer->column;
    size_t start_pos = lexer->pos;

    // hex: 0x...
    if (lexer->current == '0' && (lexer_peek(lexer) == 'x' || lexer_peek(lexer) == 'X')) {
        lexer_advance(lexer); // '0'
        lexer_advance(lexer); // 'x'
        while (isxdigit((unsigned char)lexer->current)) lexer_advance(lexer);
    }
    // binary: 0b...
    else if (lexer->current == '0' && (lexer_peek(lexer) == 'b' || lexer_peek(lexer) == 'B')) {
        lexer_advance(lexer); // '0'
        lexer_advance(lexer); // 'b'
        while (lexer->current == '0' || lexer->current == '1') lexer_advance(lexer);
    }
    // decimal
    else {
        while (isdigit((unsigned char)lexer->current)) lexer_advance(lexer);
    }

    int length = (int)(lexer->pos - start_pos);
    return lexer_make_token(TOK_NUMBER, &lexer->input[start_pos], length, line, column);
}

static Token lexer_read_identifier_or_keyword(Lexer *lexer) {
    int line = lexer->line;
    int column = lexer->column;
    size_t start_pos = lexer->pos;

    while (isalnum((unsigned char)lexer->current) || lexer->current == '_') {
        lexer_advance(lexer);
    }

    int length = (int)(lexer->pos - start_pos);
    const char *start = &lexer->input[start_pos];
    TokenType type = lexer_keyword_type(start, length);
    return lexer_make_token(type, start, length, line, column);
}

Token lexer_next_token(Lexer *lexer) {
    while (1) {
        lexer_skip_whitespace(lexer);
        if (lexer->current == '/' && (lexer_peek(lexer) == '/' || lexer_peek(lexer) == '*')) {
            lexer_skip_comment(lexer);
            continue;
        }
        break;
    }

    int line = lexer->line;
    int column = lexer->column;

    if (lexer->current == '\0') {
        return lexer_make_token(TOK_EOF, "", 0, line, column);
    }

    // string
    if (lexer->current == '"') {
        return lexer_read_string(lexer);
    }

    // number
    if (isdigit((unsigned char)lexer->current)) {
        return lexer_read_number(lexer);
    }

    // identifier / keyword
    if (isalpha((unsigned char)lexer->current) || lexer->current == '_') {
        return lexer_read_identifier_or_keyword(lexer);
    }

    // operators / punctuation
    char c = lexer->current;
    lexer_advance(lexer);

    switch (c) {
        // arithmetic
        case '+':
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_PLUS_ASSIGN, "+=", 2, line, column); }
            return lexer_make_token(TOK_PLUS, "+", 1, line, column);
        case '-':
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_MINUS_ASSIGN, "-=", 2, line, column); }
            if (lexer->current == '>') { lexer_advance(lexer); return lexer_make_token(TOK_ARROW, "->", 2, line, column); }
            return lexer_make_token(TOK_MINUS, "-", 1, line, column);
        case '*':
            return lexer_make_token(TOK_STAR, "*", 1, line, column);
        case '/':
            return lexer_make_token(TOK_SLASH, "/", 1, line, column);
        case '%':
            return lexer_make_token(TOK_PERCENT, "%", 1, line, column);

        // bitwise/logical
        case '&':
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_AND_ASSIGN, "&=", 2, line, column); }
            return lexer_make_token(TOK_AND, "&", 1, line, column);
        case '|':
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_OR_ASSIGN, "|=", 2, line, column); }
            return lexer_make_token(TOK_OR, "|", 1, line, column);
        case '^':
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_XOR_ASSIGN, "^=", 2, line, column); }
            return lexer_make_token(TOK_XOR, "^", 1, line, column);
        case '~':
            return lexer_make_token(TOK_NOT, "~", 1, line, column);
        case '!':
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_NE, "!=", 2, line, column); }
            return lexer_make_token(TOK_NOT, "!", 1, line, column);

        // comparisons / shifts / rotates
        case '<':
            if (lexer->current == '<') {
                lexer_advance(lexer);
                if (lexer->current == '<') { lexer_advance(lexer); return lexer_make_token(TOK_LROTATE, "<<<", 3, line, column); }
                return lexer_make_token(TOK_LSHIFT, "<<", 2, line, column);
            }
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_LE, "<=", 2, line, column); }
            return lexer_make_token(TOK_LT, "<", 1, line, column);

        case '>':
            if (lexer->current == '>') {
                lexer_advance(lexer);
                if (lexer->current == '>') { lexer_advance(lexer); return lexer_make_token(TOK_RROTATE, ">>>", 3, line, column); }
                return lexer_make_token(TOK_RSHIFT, ">>", 2, line, column);
            }
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_GE, ">=", 2, line, column); }
            return lexer_make_token(TOK_GT, ">", 1, line, column);

        case '=':
            if (lexer->current == '=') { lexer_advance(lexer); return lexer_make_token(TOK_EQ, "==", 2, line, column); }
            // Default: treat '=' as assignment (TOK_EQUAL exists but is not emitted in this brace-style lexer).
            return lexer_make_token(TOK_ASSIGN, "=", 1, line, column);

        // delimiters
        case '(':
            return lexer_make_token(TOK_LPAREN, "(", 1, line, column);
        case ')':
            return lexer_make_token(TOK_RPAREN, ")", 1, line, column);
        case '{':
            return lexer_make_token(TOK_LBRACE, "{", 1, line, column);
        case '}':
            return lexer_make_token(TOK_RBRACE, "}", 1, line, column);
        case '[':
            return lexer_make_token(TOK_LBRACKET, "[", 1, line, column);
        case ']':
            return lexer_make_token(TOK_RBRACKET, "]", 1, line, column);
        case ':':
            return lexer_make_token(TOK_COLON, ":", 1, line, column);
        case ';':
            return lexer_make_token(TOK_SEMICOLON, ";", 1, line, column);
        case ',':
            return lexer_make_token(TOK_COMMA, ",", 1, line, column);
        case '.':
            return lexer_make_token(TOK_DOT, ".", 1, line, column);
        case '@':
            return lexer_make_token(TOK_AT, "@", 1, line, column);

        default:
            return lexer_make_token(TOK_ERROR, "", 0, line, column);
    }
}
