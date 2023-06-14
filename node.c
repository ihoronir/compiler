#include "compiler.h"

static int children_num(NodeKind tk) {
    switch (tk) {
        // 終端記号
        case ND_CONST:
        case ND_LOCAL_VAR:
        case ND_NULL:
            return 0;

        // 非終端記号（子の個数が固定）
        case ND_RETURN:
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
        case ND_PROGRAM:
            return -1;
    }
}

Node new_node(NodeKind nk, ...) {
    int n = children_num(nk);
    if (n <= 0) error("new_node: nk が不正です");

    Node node = checkd_malloc(sizeof(*node));
    node->kind = nk;
    node->children = new_vec_with_capacity(n);

    va_list ap;
    va_start(ap, nk);

    Node node_child;
    while ((node_child = va_arg(ap, Node)) != NULL) {
        vec_push(node->children, node_child);
    }

    va_end(ap);

    if (node->children->len != n) error("new_node: 引数の数が不正です");
    return node;
}

Node new_node_const(int val) {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_CONST;
    node->val = val;
    return node;
}

Node new_node_local_var(int offset) {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_LOCAL_VAR;
    node->offset = offset;
    return node;
}

Node new_node_null() {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_NULL;
    return node;
}

Node new_node_block(Vec children) {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_BLOCK;
    node->children = children;
    return node;
}

Node new_node_func(char *name, int size, Vec children) {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_FUNC;
    node->children = children;
    node->name = name;
    node->size = size;
    return node;
}

Node new_node_program(Vec children) {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_PROGRAM;
    node->children = children;
    return node;
}

Node node_get_child(Node node, int index) {
    if (index >= node->children->len) error("node_get_child: 範囲外アクセス");
    return node->children->buf[index];
}
