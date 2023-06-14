#include "compiler.h"

#define LOCAL_VARS_BUF_INIT_CAPACITY 100

NameSpace new_namespace(NameSpace parent) {
    NameSpace name_space = checkd_malloc(sizeof(*name_space));
    name_space->items = new_vec();
    name_space->parent = parent;
    if (parent == NULL) {
        name_space->offset = 0;
    } else {
        name_space->offset = parent->offset;
    }
    return name_space;
}

static void set_size(NameSpace name_space, int size) {
    name_space->size = size;
    if (name_space->parent != NULL) set_size(name_space->parent, size);
}

static Item get_item(NameSpace name_space, char *name) {
    int i;
    for (i = 0; i < name_space->items->len; i++) {
        Item item = name_space->items->buf[i];
        if (!strcmp(item->name, name)) return item;
    }
    return NULL;
}

void name_space_def_func(NameSpace name_space, char *name) {
    if (get_item(name_space, name) != NULL)
        error("同じ名前のアイテムがあります");
    Item item = checkd_malloc(sizeof(*item));
    item->kind = IT_FUNC;
    item->name = name;
    vec_push(name_space->items, item);
}

void name_space_def_local_var(NameSpace name_space, char *name) {
    if (get_item(name_space, name) != NULL)
        error("同じ名前のアイテムがあります");
    Item item = checkd_malloc(sizeof(*item));
    item->kind = IT_LOCAL_VAR;
    item->name = name;
    item->offset = name_space->offset;
    name_space->offset += 8;
    set_size(name_space, name_space->offset);
    vec_push(name_space->items, item);
}

int name_space_get_local_var_offset(NameSpace name_space, char *name) {
    Item item = get_item(name_space, name);
    if (item == NULL) error("未定義の変数です");
    if (item->kind != IT_LOCAL_VAR) error("変数名ではありません");
    return item->offset;
}
