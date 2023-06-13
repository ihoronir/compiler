#include <stdio.h>

#include "compiler.h"

static int childs_num(NodeKind tk) {
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
            return -1;
    }
}

Node new_node(NodeKind nk, ...) {
    int n = childs_num(nk);
    if (n <= 0) error("new_node: nk が不正です");

    Node node = checkd_malloc(sizeof(*node));
    node->kind = nk;
    node->childs = new_vec_with_capacity(n);

    va_list ap;
    va_start(ap, nk);

    Node node_child;
    while ((node_child = va_arg(ap, Node)) != NULL) {
        vec_push(node->childs, node_child);
    }

    va_end(ap);

    if (node->childs->len != n) error("new_node: 引数の数が不正です");
    return node;
}

Node new_node_block(Vec childs) {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_BLOCK;
    node->childs = childs;
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
    node->kind = ND_CONST;
    node->offset = offset;
    return node;
}

Node new_node_null() {
    Node node = checkd_malloc(sizeof(*node));
    node->kind = ND_NULL;
    return node;
}

Node node_get_child(Node node, int index) {
    if (index >= node->childs->len) error("node_get_child: 範囲外アクセス");
    return node->childs->buf[index];
}
