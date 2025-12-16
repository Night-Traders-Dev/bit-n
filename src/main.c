/**
 * bit(N) Compiler - Main Entry Point
 * Complete version with full --compile support and device code generation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "parser.h"
#include "ast.h"
#include "lexer.h"
#include "../backend/codegen/codegen.h"

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
    const char *default_source = source;
    int should_free = 0;
    int do_codegen = 0;
    int verbose = 0;
    const char *input_file = NULL;

    // Parse arguments
    int i = 1;
    while (i < argc) {
        if (strcmp(argv[i], "--compile") == 0) {
            do_codegen = 1;
            i++;
        } else if (strcmp(argv[i], "--verbose") == 0) {
            verbose = 1;
            i++;
        } else if (strcmp(argv[i], "-c") == 0) {
            if (i + 1 < argc) {
                source = argv[++i];
                default_source = source;
                should_free = 0;
                i++;
            } else {
                fprintf(stderr, "Error: -c requires code argument\n");
                return 1;
            }
        } else if (argv[i][0] != '-') {
            input_file = argv[i];
            i++;
        } else {
            i++;
        }
    }

    // Load file if specified and source is still default
    if (input_file && source == default_source) {
        FILE *f = fopen(input_file, "r");
        if (!f) {
            fprintf(stderr, "Error: Cannot open file %s\n", input_file);
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

    printf("Input: %s\n\n", input_file ? input_file : "default");

    // Create lexer and tokenize
    Lexer *lexer = lexer_create(source);
    if (verbose) {
        printf("--- Lexical Analysis ---\n");
    }
    
    Token tok;
    int token_count = 0;
    
    do {
        tok = lexer_next_token(lexer);
        if (verbose) {
            token_print(&tok);
        }
        token_count++;
    } while (tok.type != TOK_EOF);
    
    lexer_free(lexer);
    
    if (verbose) {
        printf("Total tokens: %d\n\n", token_count - 1);
    }

    // Parse the tokens
    Parser *parser = parser_create(source);
    if (verbose) {
        printf("--- Parsing ---\n");
    }
    
    ASTProgram *program = parser_parse_program(parser);

    if (!parser_has_error(parser)) {
        printf("✅ Successfully parsed\n");
        printf(" Functions: %lu\n", program->function_count);
        for (size_t i = 0; i < program->function_count; i++) {
            printf(" - fn %s\n", program->functions[i]->name);
        }

        printf(" Peripherals: %lu\n", program->peripheral_count);
        for (size_t i = 0; i < program->peripheral_count; i++) {
            ASTPeripheral *periph = program->peripherals[i];
            printf(" - peripheral %s @ 0x%08X\n", periph->name, periph->base_address);
            for (size_t j = 0; j < periph->register_count; j++) {
                ASTRegister *reg = periph->registers[j];
                printf(" * register %s: %s @ offset 0x%02X\n",
                    reg->name, type_kind_name(reg->type->kind), reg->offset);
                for (size_t k = 0; k < reg->field_count; k++) {
                    ASTField *field = reg->fields[k];
                    printf(" - field %s: [%u:%u] %s\n",
                        field->name, field->start_bit, field->end_bit,
                        access_kind_name(field->access));
                }
            }
        }

        // ========== CODE GENERATION ==========
        if (do_codegen) {
            if (program->peripheral_count > 0) {
                printf("\n--- Code Generation ---\n");
                
                // Generate output filename
                const char *output_file = "generated.h";
                char output_buf[256];
                
                if (input_file) {
                    // Create output filename from input filename
                    strncpy(output_buf, input_file, sizeof(output_buf) - 1);
                    output_buf[sizeof(output_buf) - 1] = '\0';
                    char *dot = strrchr(output_buf, '.');
                    if (dot) {
                        strcpy(dot, ".h");
                    } else {
                        strcat(output_buf, ".h");
                    }
                    output_file = output_buf;
                }

                printf("Generating C code to: %s\n", output_file);

                // Initialize code generator
                CodegenContext *ctx = codegen_init(output_file, "arm-cortex-m0");
                if (!ctx) {
                    fprintf(stderr, "Error: Failed to initialize code generator\n");
                    parser_free(parser);
                    ast_free_program(program);
                    if (should_free) free((void *)source);
                    return 1;
                }

                // Generate code
                if (codegen_generate(ctx, program) == 0) {
                    printf("✅ Successfully generated C code\n");
                } else {
                    fprintf(stderr, "❌ Code generation failed\n");
                    codegen_cleanup(ctx);
                    parser_free(parser);
                    ast_free_program(program);
                    if (should_free) free((void *)source);
                    return 1;
                }

                codegen_cleanup(ctx);
            } else {
                printf("\n⚠️  --compile flag specified but no peripherals found.\n");
                printf("   Device files must contain 'peripheral' definitions.\n");
                printf("\n   Example:\n");
                printf("     peripheral UART {\n");
                printf("         base_address: 0x40000000\n");
                printf("         register CTRL { ... }\n");
                printf("     }\n");
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
