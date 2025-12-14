#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "type_system.h"


int main(int argc, char *argv[]) {
    printf("=== bit(N) Compiler ===\n");
    
    const char *source = "fn main() -> u32 { return 42; }";
    int should_free = 0;
    
    if (argc > 2 && strcmp(argv[1], "-c") == 0) {
        source = argv[2];
        should_free = 0;
    } else if (argc > 1) {
        FILE *f = fopen(argv[1], "r");
        if (!f) {
            fprintf(stderr, "Error: Cannot open file %s\n", argv[1]);
            return 1;
        }
        fseek(f, 0, SEEK_END);
        long size = ftell(f);
        fseek(f, 0, SEEK_SET);
        
        char *file_source = malloc(size + 1);
        if (!file_source) {
            fprintf(stderr, "Error: Memory allocation failed\n");
            fclose(f);
            return 1;
        }
        
        size_t bytes_read = fread(file_source, 1, (size_t)size, f);
        file_source[bytes_read] = '\0';
        fclose(f);
        
        source = file_source;
        should_free = 1;
    }
    
    printf("Input: %s\n\n", argc > 1 ? (argc > 2 && strcmp(argv[1], "-c") == 0 ? "command-line" : argv[1]) : "default");
    
    // ============================================
    // PHASE 1: LEXICAL ANALYSIS
    // ============================================
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
    
    // ============================================
    // PHASE 1: SYNTAX ANALYSIS (PARSING)
    // ============================================
    Parser *parser = parser_create(source);
    printf("--- Parsing ---\n");
    ASTProgram *program = parser_parse_program(parser);
    
    if (parser_has_error(parser)) {
        printf("❌ Parsing failed\n");
        parser_free(parser);
        ast_free_program(program);
        if (should_free) {
            free((void *)source);
        }
        return 1;
    }
    
    parser_free(parser);
    printf("✅ Successfully parsed %lu functions\n", program->function_count);
    for (size_t i = 0; i < program->function_count; i++) {
        printf("  - %s() -> %s\n", 
               program->functions[i]->name,
               program->functions[i]->return_type ? 
               type_kind_to_string(program->functions[i]->return_type->kind) : "unknown");
    }
    printf("\n=== Compilation Complete ===\n");
    
    // ============================================
    // PHASE 2: SEMANTIC ANALYSIS (TYPE CHECKING)
    // ============================================
    printf("\n--- Type Checking ---\n");
    
    int type_errors = 0;
    
    // Verify function return types are valid
    for (size_t i = 0; i < program->function_count; i++) {
        ASTFunctionDef *func = program->functions[i];
        if (!func->return_type) {
            fprintf(stderr, "Warning: Function '%s' has no return type\n", func->name);
            type_errors++;
        }
    }
    
    if (type_errors == 0) {
        printf("✅ Type checking passed\n");
    } else {
        printf("❌ Type checking failed with %d error(s)\n", type_errors);
        ast_free_program(program);
        if (should_free) {
            free((void *)source);
        }
        return 1;
    }
    
    // ============================================
    // CLEANUP
    // ============================================
    ast_free_program(program);
    
    if (should_free) {
        free((void *)source);
    }
    
    return 0;
}
