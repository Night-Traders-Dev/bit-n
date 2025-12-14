#ifndef SYMBOL_TABLE_H
#define SYMBOL_TABLE_H

#include "ast.h"
#include <stddef.h>

// A symbol is a named entity (variable, parameter, function)
typedef struct {
    const char *name;
    Type *type;
    int is_parameter;      // 1 if parameter, 0 if local variable
    int is_initialized;    // 1 if initialized, 0 otherwise
} Symbol;

// Scope represents a block of code with its own variable space
typedef struct Scope {
    Symbol *symbols;
    size_t symbol_count;
    size_t capacity;
    struct Scope *parent;  // Enclosing scope for nested blocks
} Scope;

// Symbol table manages all scopes
typedef struct {
    Scope *current_scope;
    Scope *global_scope;
} SymbolTable;

// API - Lifecycle
SymbolTable *symbol_table_create(void);
void symbol_table_free(SymbolTable *table);

// API - Scope management
void symbol_table_push_scope(SymbolTable *table);
void symbol_table_pop_scope(SymbolTable *table);

// API - Symbol management
int symbol_table_add_symbol(SymbolTable *table, const char *name, Type *type, int is_param);
Symbol *symbol_table_lookup(SymbolTable *table, const char *name);
Symbol *symbol_table_lookup_local(SymbolTable *table, const char *name);
int symbol_table_is_defined(SymbolTable *table, const char *name);
int symbol_table_is_defined_local(SymbolTable *table, const char *name);

#endif // SYMBOL_TABLE_H
