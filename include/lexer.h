#ifndef LEXER_H
#define LEXER_H

#include "token.h"

typedef struct {
    const char *input;
    size_t pos;
    int line;
    int column;
    char current;
} Lexer;

Lexer *lexer_create(const char *input);
void lexer_free(Lexer *lexer);
Token lexer_next_token(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);
void lexer_skip_comment(Lexer *lexer);

#endif // LEXER_H
