#include "compiler.h"

Type new_type_int() {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_INT;
    return type;
}

Type new_type_func() {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_FUNC;
    return type;
}
