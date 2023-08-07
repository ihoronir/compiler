#include "compiler.h"

static int children_num(NodeKind kind) {
    switch (kind) {
        // 終端記号
        case ND_CONST_INT:
        case ND_LOCAL_VAR:
        case ND_NULL:
            return 0;

        // 非終端記号（子の個数が固定）
        case ND_RETURN:
        case ND_ADDR:
        case ND_DEREF:
            return 1;
        case ND_MUL:
        case ND_DIV:
        case ND_ADD:
        case ND_SUB:
        case ND_LESS:
        case ND_LESS_OR_EQUAL:
        case ND_EQUAL:
        case ND_NOT_EQUAL:
        case ND_ASSIGN:
        case ND_IF:
        case ND_WHILE:
            return 2;
        case ND_IF_ELSE:
            return 3;
        case ND_FOR:
            return 4;

        // 非終端記号（子の個数が可変）
        case ND_BLOCK:
        case ND_FUNC:
        case ND_CALL:
        case ND_PROGRAM:
            return -1;
    }

    error("children_num: unreacnable");
    return 0;
}

UntypedNode new_untyped_node(NodeKind kind, ...) {
    int n = children_num(kind);
    assert(n > 0);

    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = kind;
    untyped_node->children = new_vec_with_capacity(n);

    va_list ap;
    va_start(ap, kind);

    UntypedNode untyped_node_child;
    while ((untyped_node_child = va_arg(ap, UntypedNode)) != NULL) {
        vec_push(untyped_node->children, untyped_node_child);
    }

    va_end(ap);
    assert(untyped_node->children->len == n);
    return untyped_node;
}

UntypedNode new_untyped_node_const_int(int val_int) {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_CONST_INT;
    untyped_node->val_int = val_int;
    return untyped_node;
}

UntypedNode new_untyped_node_local_var(Scope scope, char *name) {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_LOCAL_VAR;
    untyped_node->item = scope_get_item(IT_LOCAL_VAR, scope, name);
    return untyped_node;
}

UntypedNode new_untyped_node_local_var_with_def(Scope scope, Type type,
                                                char *name) {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_LOCAL_VAR;
    untyped_node->item = scope_def_local_var(scope, type, name);
    return untyped_node;
}

UntypedNode new_untyped_node_func(Scope scope, Type type, char *name,
                                  Vec children) {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_FUNC;
    untyped_node->children = children;
    untyped_node->item = scope_def_func(scope, type, name);
    return untyped_node;
}

UntypedNode new_untyped_node_null() {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_NULL;
    return untyped_node;
}

UntypedNode new_untyped_node_block(Vec children) {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_BLOCK;
    untyped_node->children = children;
    return untyped_node;
}

UntypedNode new_untyped_node_call(Scope scope, char *name, Vec children) {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_CALL;
    untyped_node->children = children;
    untyped_node->item = scope_get_item(IT_FUNC, scope, name);
    return untyped_node;
}

UntypedNode new_untyped_node_program(Vec children) {
    UntypedNode untyped_node = checked_malloc(sizeof(*untyped_node));
    untyped_node->kind = ND_PROGRAM;
    untyped_node->children = children;
    return untyped_node;
}

UntypedNode untyped_node_get_child(UntypedNode untyped_node, int index) {
    if (index >= untyped_node->children->len)
        error("node_get_child: 範囲外アクセス");
    return untyped_node->children->buf[index];
}
