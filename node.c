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
}

Node new_node(NodeKind kind, ...) {
    int n = children_num(kind);
    if (n <= 0) error("new_node: kind が不正です");

    Node node = checked_malloc(sizeof(*node));
    node->kind = kind;
    node->children = new_vec_with_capacity(n);

    va_list ap;
    va_start(ap, kind);

    Node node_child;
    while ((node_child = va_arg(ap, Node)) != NULL) {
        vec_push(node->children, node_child);
    }

    va_end(ap);

    if (node->children->len != n) error("new_node: 引数の数が不正です");
    return node;
}

Node new_node_const_int(int val_int) {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_CONST_INT;
    node->val_int = val_int;
    return node;
}

Node new_node_local_var(Scope scope, char *name) {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_LOCAL_VAR;
    node->item = scope_get_item(IT_LOCAL_VAR, scope, name);
    return node;
}

Node new_node_local_var_with_def(Scope scope, Type type, char *name) {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_LOCAL_VAR;
    node->item = scope_def_local_var(scope, type, name);
    return node;
}

Node new_node_func(Scope scope, Type type, char *name, Vec children) {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_FUNC;
    node->children = children;
    node->item = scope_def_func(scope, type, name);
    return node;
}

Node new_node_null() {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_NULL;
    return node;
}

Node new_node_block(Vec children) {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_BLOCK;
    node->children = children;
    return node;
}

Node new_node_call(Scope scope, char *name, Vec children) {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_CALL;
    node->children = children;
    node->item = scope_get_item(IT_FUNC, scope, name);
    return node;
}

Node new_node_program(Vec children) {
    Node node = checked_malloc(sizeof(*node));
    node->kind = ND_PROGRAM;
    node->children = children;
    return node;
}

Node node_get_child(Node node, int index) {
    if (index >= node->children->len) error("node_get_child: 範囲外アクセス");
    return node->children->buf[index];
}
