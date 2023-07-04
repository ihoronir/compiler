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

int type_cmp(Type type1, Type type2) {
    if (type1->kind != type2->kind) return 0;
    switch (type1->kind) {
        case TY_INT:
            return 1;
        case TY_PTR:
            return type_cmp(type1->ptr_to, type2->ptr_to);
        default:
            error("type_cmp: unimplemented");
            error("type_cmp: unreacnable");
            return 0;
    }
}
