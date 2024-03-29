#include "compiler.h"

#define LOCAL_VARS_BUF_INIT_CAPACITY 100

Scope new_scope_global() {
    Scope scope = checked_malloc(sizeof(*scope));
    scope->parent = NULL;
    scope->items = new_vec();
    scope->func = NULL;
    return scope;
}

Scope new_scope_func(Scope parent, Item item) {
    Scope scope = checked_malloc(sizeof(*scope));
    scope->parent = parent;
    scope->items = new_vec();
    scope->func = item;
    return scope;
}

Scope new_scope(Scope parent) {
    Scope scope = checked_malloc(sizeof(*scope));
    scope->parent = parent;
    scope->items = new_vec();
    scope->func = parent->func;
    return scope;
}

// 現在のスコープからのみ検索
static Item get_item(Scope scope, char *name) {
    for (int i = 0; i < scope->items->len; i++) {
        Item item = scope->items->buf[i];
        if (!strcmp(name, item->name)) return item;
    }
    return NULL;
}

// 親スコープも含めて検索
Item scope_get_item(Scope scope, char *name) {
    Item item = get_item(scope, name);
    if (item == NULL) {
        if (scope->parent == NULL) error("未定義の変数です");
        return scope_get_item(scope->parent, name);
    }
    return item;
}

Item scope_def_func(Scope scope, Type type, char *name) {
    if (get_item(scope, name) != NULL) error("同じ名前のアイテムがあります");
    Item item = new_item_func(type, name);
    vec_push(scope->items, item);
    return item;
}

Item scope_def_global_var(Scope scope, Type type, char *name) {
    if (get_item(scope, name) != NULL) error("同じ名前のアイテムがあります");
    Item item = new_item_global_var(type, name);
    vec_push(scope->items, item);
    return item;
}

Item scope_def_local_var(Scope scope, Type type, char *name) {
    if (get_item(scope, name) != NULL) error("同じ名前のアイテムがあります");
    int size = type_size(type);
    scope->func->size += size > 8 ? size : 8;
    int offset = scope->func->size;
    Item item = new_item_local_var(type, name, offset);
    vec_push(scope->items, item);
    return item;
}
