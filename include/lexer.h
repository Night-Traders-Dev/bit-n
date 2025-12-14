#ifndef LEXER_H
#define LEXER_H

#include "token.h"

#define MAX_INDENT_DEPTH 64

typedef struct {
    // Input tracking
    const char *input;
    size_t pos;
    int line;
    int column;
    char current;
    
    // ✅ NEW: Indentation tracking for Nim-style blocks
    int indent_stack[MAX_INDENT_DEPTH];  // Stack of indent levels (space count)
    int indent_depth;                     // Current position in indent_stack
    int pending_dedents;                  // Number of DEDENT tokens to emit
    int indent_size;                      // Detected indent unit (2, 4, etc)
    int last_line_indent;                 // Indentation of previous line
    int at_line_start;                    // Flag: are we at start of new line?
} Lexer;

Lexer *lexer_create(const char *input);
void lexer_free(Lexer *lexer);

Token lexer_next_token(Lexer *lexer);
void lexer_skip_whitespace(Lexer *lexer);
void lexer_skip_comment(Lexer *lexer);

#endif // LEXER_H
