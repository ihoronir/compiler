#include "compiler.h"

Item new_item_func(Type type, char *name) {
    Item item = checked_malloc(sizeof(*item));
    item->kind = IT_FUNC;
    item->type = type;
    item->name = name;
    item->size = 0;
    return item;
}

Item new_item_global_var(Type type, char *name) {
    Item item = checked_malloc(sizeof(*item));
    item->kind = IT_GLOBAL_VAR;
    item->type = type;
    item->name = name;
    return item;
}

Item new_item_local_var(Type type, char *name, int offset) {
    Item item = checked_malloc(sizeof(*item));
    item->kind = IT_LOCAL_VAR;
    item->type = type;
    item->name = name;
    item->offset = offset;
    return item;
}
