#include "compiler.h"

Type new_type_int() {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_INT;
    return type;
}

Type new_type_ptr(Type ptr_to) {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_PTR;
    type->ptr_to = ptr_to;
    return type;
}

Type new_type_func() {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_FUNC;
    return type;
}
