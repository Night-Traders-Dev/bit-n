#include "../include/type_system.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

Type *type_clone(Type *type) {
    if (!type) return NULL;
    Type *clone = (Type *)malloc(sizeof(Type));
    clone->kind = type->kind;
    return clone;
}

void type_free(Type *type) {
    if (type) {
        free(type);
    }
}

int type_equal(Type *a, Type *b) {
    if (!a || !b) {
        return (a == b);  // Both NULL = equal
    }
    
    return a->kind == b->kind;
}

int type_compatible(Type *target, Type *source) {
    // For now, types must be exactly equal
    // Future: add implicit conversions (u8 -> u32, etc.)
    return type_equal(target, source);
}

int type_is_numeric(Type *type) {
    if (!type) return 0;
    return type_is_integer(type);
}

int type_is_integer(Type *type) {
    if (!type) return 0;
    
    switch (type->kind) {
        case TYPE_U8:
        case TYPE_U16:
        case TYPE_U32:
        case TYPE_U64:
        case TYPE_I8:
        case TYPE_I16:
        case TYPE_I32:
        case TYPE_I64:
            return 1;
        default:
            return 0;
    }
}

int type_is_void(Type *type) {
    if (!type) return 0;
    return type->kind == TYPE_VOID;
}

int type_is_error(Type *type) {
    if (!type) return 1;  // NULL is error
    return 0;  // No error type in current system
}

const char *type_kind_to_string(TypeKind kind) {
    switch (kind) {
        case TYPE_VOID:  return "void";
        case TYPE_U8:    return "u8";
        case TYPE_U16:   return "u16";
        case TYPE_U32:   return "u32";
        case TYPE_U64:   return "u64";
        case TYPE_I8:    return "i8";
        case TYPE_I16:   return "i16";
        case TYPE_I32:   return "i32";
        case TYPE_I64:   return "i64";
        default:         return "unknown";
    }
}

const char *type_to_string(Type *type) {
    if (!type) return "null";
    return type_kind_to_string(type->kind);
}

uint64_t type_get_size(TypeKind kind) {
    switch (kind) {
        case TYPE_U8:
        case TYPE_I8:
            return 1;
        case TYPE_U16:
        case TYPE_I16:
            return 2;
        case TYPE_U32:
        case TYPE_I32:
            return 4;
        case TYPE_U64:
        case TYPE_I64:
            return 8;
        case TYPE_VOID:
            return 0;
        default:
            return 0;
    }
}

Type *type_from_kind(TypeKind kind) {
    Type *type = (Type *)malloc(sizeof(Type));
    type->kind = kind;
    return type;
}

TypeKind type_kind_from_string(const char *name, int length) {
    // Use length to avoid needing null-terminated string
    
    if (length == 2) {
        if (strncmp(name, "u8", 2) == 0) return TYPE_U8;
        if (strncmp(name, "i8", 2) == 0) return TYPE_I8;
    }
    
    if (length == 3) {
        if (strncmp(name, "u16", 3) == 0) return TYPE_U16;
        if (strncmp(name, "u32", 3) == 0) return TYPE_U32;
        if (strncmp(name, "u64", 3) == 0) return TYPE_U64;
        if (strncmp(name, "i16", 3) == 0) return TYPE_I16;
        if (strncmp(name, "i32", 3) == 0) return TYPE_I32;
        if (strncmp(name, "i64", 3) == 0) return TYPE_I64;
    }
    
    if (length == 4) {
        if (strncmp(name, "void", 4) == 0) return TYPE_VOID;
    }
    
    return TYPE_VOID;  // Default to void instead of error
}
