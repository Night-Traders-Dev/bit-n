/**
 * bit(N) Compiler - Linker Script Generator
 */

#ifndef BITN_LINKER_GEN_H
#define BITN_LINKER_GEN_H

#include <stdio.h>
#include <stdint.h>

typedef struct {
    char *name;
    uint32_t base;
    uint32_t size;
    char *type;
} MemoryRegion;

typedef struct {
    FILE *output;
    MemoryRegion *regions;
    int region_count;
    char *target;
    uint32_t stack_size;
    uint32_t heap_size;
} LinkerContext;

LinkerContext* linker_init(const char *output_file, const char *target);
int linker_add_region(LinkerContext *ctx, const char *name, 
                     uint32_t base, uint32_t size, const char *type);
int linker_generate(LinkerContext *ctx);
void linker_cleanup(LinkerContext *ctx);

#endif // BITN_LINKER_GEN_H
