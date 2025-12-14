#ifndef TYPE_SYSTEM_H
#define TYPE_SYSTEM_H

#include "ast.h"

// Type operations using existing Type from ast.h
Type *type_clone(Type *type);
void type_free(Type *type);

// Type comparison and conversion
int type_equal(Type *a, Type *b);
int type_compatible(Type *target, Type *source);
int type_is_numeric(Type *type);
int type_is_integer(Type *type);
int type_is_void(Type *type);
int type_is_error(Type *type);

// Type information
const char *type_to_string(Type *type);
const char *type_kind_to_string(TypeKind kind);
uint64_t type_get_size(TypeKind kind);

// Type conversion
Type *type_from_kind(TypeKind kind);
TypeKind type_kind_from_string(const char *name, int length);

#endif // TYPE_SYSTEM_H
