#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "ast.h"
#include "lexer.h"

static const char *access_kind_name(AccessKind access) {
    switch (access) {
        case ACCESS_RO: return "ro";
        case ACCESS_WO: return "wo";
        case ACCESS_RW: return "rw";
        case ACCESS_W1C: return "w1c";
        default: return "?";
    }
}

static const char *type_kind_name(TypeKind kind) {
    switch (kind) {
        case TYPE_VOID: return "void";
        case TYPE_U8: return "u8";
        case TYPE_U16: return "u16";
        case TYPE_U32: return "u32";
        case TYPE_U64: return "u64";
        case TYPE_I8: return "i8";
        case TYPE_I16: return "i16";
        case TYPE_I32: return "i32";
        case TYPE_I64: return "i64";
        default: return "?";
    }
}

int main(int argc, char *argv[]) {
    printf("=== bit(N) Compiler with DSL Support ===\n\n");
    
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
    
    printf("Input: %s\n\n", argc > 1 ? argv[1] : "default");
    
    // Create lexer and tokenize
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
    
    // Parse the tokens
    Parser *parser = parser_create(source);
    printf("--- Parsing ---\n");
    
    ASTProgram *program = parser_parse_program(parser);
    
    if (!parser_has_error(parser)) {
        printf("✅ Successfully parsed\n");
        printf("   Functions: %lu\n", program->function_count);
        for (size_t i = 0; i < program->function_count; i++) {
            printf("     - fn %s\n", program->functions[i]->name);
        }
        
        printf("   Peripherals: %lu\n", program->peripheral_count);
        for (size_t i = 0; i < program->peripheral_count; i++) {
            ASTPeripheral *periph = program->peripherals[i];
            printf("     - peripheral %s @ 0x%08X\n", periph->name, periph->base_address);
            
            for (size_t j = 0; j < periph->register_count; j++) {
                ASTRegister *reg = periph->registers[j];
                printf("       * register %s: %s @ offset 0x%02X\n",
                       reg->name, type_kind_name(reg->type->kind), reg->offset);
                
                for (size_t k = 0; k < reg->field_count; k++) {
                    ASTField *field = reg->fields[k];
                    printf("         - field %s: [%u:%u] %s\n",
                           field->name, field->start_bit, field->end_bit,
                           access_kind_name(field->access));
                }
            }
        }
        
        printf("\n=== Compilation Successful ===\n");
    } else {
        printf("❌ Parsing failed\n");
    }
    
    // Cleanup
    parser_free(parser);
    ast_free_program(program);
    
    if (should_free) {
        free((void *)source);
    }
    
    return 0;
}
