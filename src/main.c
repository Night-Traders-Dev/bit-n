#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "ast.h"
#include "lexer.h"


int main(int argc, char *argv[]) {
    printf("=== bit(N) Compiler ===\n");
    
    const char *source = "fn main() -> u32 { return 42; }";
    
    if (argc > 2 && strcmp(argv[1], "-c") == 0) {
        source = argv[2];
    } else if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
            return 1;
        }
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        source = malloc(size + 1);
        size_t bytes_read = fread((void *)source, 1, (size_t)size, f);
        ((char *)source)[bytes_read] = '\0';
        fclose(f);
    }
    
    printf("Input: %s\n\n", argc > 1 ? argv[1] : "-c");
    
    Lexer *lexer = lexer_create(source);
    printf("--- Lexical Analysis ---\n");
    Token tok;
    int token_count = 0;
    do {
        tok = lexer_next_token(lexer);
        token_print(&tok);
        token_count++;
    } while (tok.type != TOK_EOF);
    lexer_free(lexer);
    printf("Total tokens: %d\n\n", token_count - 1);
    
    Parser *parser = parser_create(source);
    printf("--- Parsing ---\n");
    ASTProgram *program = parser_parse_program(parser);
    
    if (!parser_has_error(parser)) {
        printf("Successfully parsed %lu functions\n", program->function_count);
        for (size_t i = 0; i < program->function_count; i++) {
            printf("  - %s\n", program->functions[i]->name);
        }
        printf("\n=== Compilation Complete ===\n");
    } else {
        printf("Parsing failed\n");
    }
    
    parser_free(parser);
    ast_free_program(program);
    
    return 0;
}
