#include "symbol_table.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define INITIAL_SCOPE_CAPACITY 16

// Create a new scope
static Scope *scope_create(Scope *parent) {
    Scope *scope = malloc(sizeof(Scope));
    scope->symbols = malloc(INITIAL_SCOPE_CAPACITY * sizeof(Symbol));
    scope->symbol_count = 0;
    scope->capacity = INITIAL_SCOPE_CAPACITY;
    scope->parent = parent;
    return scope;
}

// Free a scope and its symbols
static void scope_free(Scope *scope) {
    if (!scope) return;
    
    // ✅ Free each symbol's copied name
    for (size_t i = 0; i < scope->symbol_count; i++) {
        free((void *)scope->symbols[i].name);
        if (scope->symbols[i].type) {
            type_free(scope->symbols[i].type);
        }
    }
    
    free(scope->symbols);
    free(scope);
}

// Create symbol table
SymbolTable *symbol_table_create(void) {
    SymbolTable *table = malloc(sizeof(SymbolTable));
    table->global_scope = scope_create(NULL);
    table->current_scope = table->global_scope;
    return table;
}

// Free symbol table
void symbol_table_free(SymbolTable *table) {
    if (!table) return;
    
    // Free all scopes starting from current
    while (table->current_scope != table->global_scope) {
        symbol_table_pop_scope(table);
    }
    
    scope_free(table->global_scope);
    free(table);
}

// Push a new scope (entering a block)
void symbol_table_push_scope(SymbolTable *table) {
    if (!table) return;
    
    Scope *new_scope = scope_create(table->current_scope);
    table->current_scope = new_scope;
}

// Pop a scope (exiting a block)
void symbol_table_pop_scope(SymbolTable *table) {
    if (!table || !table->current_scope) {
        table->current_scope = table->global_scope;
        return;
    }
    
    // Can't pop global scope
    if (table->current_scope == table->global_scope) {
        return;
    }
    
    Scope *old_scope = table->current_scope;
    table->current_scope = old_scope->parent;
    scope_free(old_scope);
}

// Add a symbol to the current scope
// Returns 1 on success, 0 if symbol already exists in current scope
int symbol_table_add_symbol(SymbolTable *table, const char *name, Type *type, int is_param) {
    if (!table || !table->current_scope || !name || !type) {
        return 0;
    }
    
    // Check for redefinition in current scope only
    if (symbol_table_is_defined_local(table, name)) {
        fprintf(stderr, "Error: symbol '%s' already defined in this scope\n", name);
        return 0;
    }
    
    // Resize if necessary
    if (table->current_scope->symbol_count >= table->current_scope->capacity) {
        table->current_scope->capacity *= 2;
        table->current_scope->symbols = realloc(table->current_scope->symbols, 
                                                 table->current_scope->capacity * sizeof(Symbol));
    }
    
    // Add symbol with copied name
    size_t idx = table->current_scope->symbol_count;
    
    // ✅ FIX: Copy the name string instead of storing pointer
    if (name) {
        table->current_scope->symbols[idx].name = malloc(strlen(name) + 1);
        strcpy((char *)table->current_scope->symbols[idx].name, name);
    } else {
        table->current_scope->symbols[idx].name = NULL;
    }
    
    table->current_scope->symbols[idx].type = type;
    table->current_scope->symbols[idx].is_parameter = is_param;
    table->current_scope->symbols[idx].is_initialized = 1;
    table->current_scope->symbol_count++;
    
    return 1;
}

// Lookup symbol (searches current scope and all parent scopes)
Symbol *symbol_table_lookup(SymbolTable *table, const char *name) {
    if (!table || !name) return NULL;
    
    Scope *scope = table->current_scope;
    while (scope) {
        for (size_t i = 0; i < scope->symbol_count; i++) {
            if (strcmp(scope->symbols[i].name, name) == 0) {
                return &scope->symbols[i];
            }
        }
        scope = scope->parent;
    }
    
    return NULL;  // Not found in any scope
}

// Lookup symbol in current scope only (no parent lookup)
Symbol *symbol_table_lookup_local(SymbolTable *table, const char *name) {
    if (!table || !table->current_scope || !name) return NULL;
    
    for (size_t i = 0; i < table->current_scope->symbol_count; i++) {
        if (strcmp(table->current_scope->symbols[i].name, name) == 0) {
            return &table->current_scope->symbols[i];
        }
    }
    
    return NULL;  // Not found in current scope
}

// Check if symbol is defined in any scope
int symbol_table_is_defined(SymbolTable *table, const char *name) {
    return symbol_table_lookup(table, name) != NULL;
}

// Check if symbol is defined in current scope only
int symbol_table_is_defined_local(SymbolTable *table, const char *name) {
    return symbol_table_lookup_local(table, name) != NULL;
}
