#include "compiler.h"

void check(Node node) {
    switch (node->kind) {
        case ND_PROGRAM:
            for (int i = 0; i < node->children->len; i++) {
                Node child = node_get_child(node, 1);
                if (child->kind != ND_FUNC)
                    error("ND_PROGRAM に ND_FUNC ではない子があります");
                check(child);
            }
            break;

        case ND_FUNC:
            for (int i = 0; i < node->children->len - 1; i++) {
                Node child = node_get_child(node, 1);
                if (child->kind != ND_LOCAL_VAR)
                    error("ND_FUNC の引数が ND_LOCAL_VAR ではありません");
                check(child);
            }
            Node func_body = node_get_child(node, node->children->len - 1);
            if (func_body->kind != ND_BLOCK)
                error("ND_FUNC の本体が ND_BLOCK ではありません");
            check(func_body);
            break;

        default:
            break;
    }
}
