#ifndef BITN_LEXER_H
#define BITN_LEXER_H

#include "token.h"
#include <stddef.h>

typedef struct {
    const char *input;
    size_t pos;
    size_t line;
    size_t column;
    char current;
} Lexer;

Lexer *lexer_create(const char *input);
void lexer_free(Lexer *lexer);
Token lexer_next_token(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);
void lexer_skip_comment(Lexer *lexer);

#endif
