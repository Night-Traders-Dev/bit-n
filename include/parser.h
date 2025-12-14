#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "ast.h"

typedef struct {
    Lexer *lexer;
    Token current;
    Token peek;
    int error;
} Parser;

Parser *parser_create(const char *input);
void parser_free(Parser *parser);
void parser_error(Parser *parser, const char *message);
int parser_has_error(Parser *parser);
ASTProgram *parser_parse_program(Parser *parser);

#endif // PARSER_H
