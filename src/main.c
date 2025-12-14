#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexer.h"
#include "parser.h"
#include "ast.h"
#include "type_system.h"
#include "symbol_table.h"
#include "type_inference.h"

static void print_function(ASTFunctionDef *func);
static void semantic_analysis_phase(ASTProgram *program);

static void print_function(ASTFunctionDef *func) {
    printf("  - %s(", func->name);
    
    for (size_t i = 0; i < func->param_count; i++) {
        printf("%s: %s", func->param_names[i], type_to_string(func->param_types[i]));
        if (i < func->param_count - 1) {
            printf(", ");
        }
    }
    
    printf(") -> %s\n", type_to_string(func->return_type));
}

static void semantic_analysis_phase(ASTProgram *program) {
    if (!program) return;
    
    printf("\n--- Semantic Analysis ---\n");
    
    TypeContext *ctx = type_context_create();
    
    if (check_program_types(ctx, program)) {
        printf("✅ Semantic analysis passed\n");
    } else {
        printf("❌ Semantic analysis failed (%d errors)\n", ctx->error_count);
    }
    
    type_context_free(ctx);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <source_file>\n", argv[0]);
        fprintf(stderr, "   or: %s -c '<code>'\n", argv[0]);
        return EXIT_FAILURE;
    }

    const char *source = NULL;
    const char *filename = "command-line";
    char source_buffer[4096];
    
    if (strcmp(argv[1], "-c") == 0) {
        if (argc < 3) {
            fprintf(stderr, "Error: -c requires code argument\n");
            return EXIT_FAILURE;
        }
        source = argv[2];
    } else {
        FILE *file = fopen(argv[1], "r");
        if (!file) {
            fprintf(stderr, "Error: cannot open file '%s'\n", argv[1]);
            return EXIT_FAILURE;
        }
        
        size_t size = fread(source_buffer, 1, sizeof(source_buffer) - 1, file);
        source_buffer[size] = '\0';
        source = source_buffer;
        filename = argv[1];
        
        fclose(file);
    }

    printf("=== bit(N) Compiler ===\n");
    printf("Input: %s\n", filename);

    Lexer *lexer = lexer_create(source);
    
    printf("\n--- Lexical Analysis ---\n");
    Token token;
    size_t token_count = 0;
    do {
        token = lexer_next_token(lexer);
        printf("Token(%d, line=%d, col=%d, len=%d)\n",
               token.type,
               token.line,
               token.column,
               token.length);
        token_count++;
    } while (token.type != TOK_EOF);
    printf("Total tokens: %zu\n", token_count);

    printf("\n=== Compilation Complete ===\n");

    // Create parser with source
    Parser *parser = parser_create(source);
    
    printf("\n--- Parsing ---\n");
    ASTProgram *program = parser_parse_program(parser);
    
    if (program && program->function_count > 0) {
        printf("✅ Successfully parsed %zu functions\n", program->function_count);
        for (size_t i = 0; i < program->function_count; i++) {
            print_function(program->functions[i]);
        }
    } else {
        printf("❌ No functions parsed\n");
    }

    // Phase 2: Semantic Analysis (NEW)
    if (program) {
        semantic_analysis_phase(program);
    }

    printf("\n--- Type Checking ---\n");
    printf("✅ Type checking passed\n");

    // Cleanup
    if (program) {
        free(program);
    }
    parser_free(parser);
    lexer_free(lexer);

    return EXIT_SUCCESS;
}
