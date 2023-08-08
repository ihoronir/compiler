#include "compiler.h"

int type_size(Type type) {
    switch (type->kind) {
        case TY_INT:
            return 4;
        case TY_PTR:
            return 8;
        default:
            error("unimplemented");
    }
}

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

Type new_type_func(Type returning) {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_FUNC;
    type->returning = returning;
    return type;
}

int type_is_equal(Type type1, Type type2) {
    if (type1->kind != type2->kind) return 0;
    switch (type1->kind) {
        case TY_INT:
            return 1;
        case TY_PTR:
            return type_is_equal(type1->ptr_to, type2->ptr_to);
        default:
            error("type_is_equal: unimplemented");
            error("type_is_equal: unreacnable");
            return 0;
    }
}
