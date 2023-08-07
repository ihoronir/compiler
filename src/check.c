#include "compiler.h"

TypedNode to_typed(UntypedNode untyped_node) {
    switch (untyped_node->kind) {
        case ND_PROGRAM: {
            Vec typed_children =
                new_vec_with_capacity(untyped_node->children->len);

            for (int i = 0; i < untyped_node->children->len; i++) {
                UntypedNode untyped_child =
                    untyped_node_get_child(untyped_node, 1);
                assert(untyped_child->kind == ND_FUNC);

                vec_push(typed_children, to_typed(untyped_node));
            }

            TypedNode typed_node = checked_malloc(sizeof(*typed_node));
            typed_node->kind = ND_PROGRAM;
            typed_node->children = typed_children;
            return typed_node;
        }

        case ND_FUNC: {
            Vec typed_children =
                new_vec_with_capacity(untyped_node->children->len);

            for (int i = 0; i < untyped_node->children->len - 1; i++) {
                UntypedNode untyped_child =
                    untyped_node_get_child(untyped_node, 1);
                assert(untyped_child->kind == ND_LOCAL_VAR);
                vec_push(typed_children, to_typed(untyped_child));
            }

            UntypedNode untyped_func_body = untyped_node_get_child(
                untyped_node, untyped_node->children->len - 1);
            assert(untyped_func_body->kind == ND_BLOCK);
            vec_push(typed_children, to_typed(untyped_func_body));

            TypedNode typed_node = checked_malloc(sizeof(*typed_node));
            typed_node->kind = ND_FUNC;
            typed_node->children = typed_children;
            typed_node->item = untyped_node->item;
            return typed_node;
        }

        case ND_BLOCK: {
            Vec children = new_vec_with_capacity(untyped_node->children->len);

            for (int i = 0; i < untyped_node->children->len; i++) {
                UntypedNode child = untyped_node_get_child(untyped_node, i);
                assert(child->kind == ND_NULL || child->kind == ND_BLOCK ||
                       child->kind == ND_IF || child->kind == ND_IF_ELSE ||
                       child->kind == ND_WHILE || child->kind == ND_FOR ||
                       child->kind == ND_RETURN);
                vec_push(children, child);
            }

            TypedNode typed_node = checked_malloc(sizeof(*typed_node));
            typed_node->kind = ND_BLOCK;
            typed_node->children = children;
            return typed_node;
        }

        case ND_CONST_INT: {
            TypedNode typed_node = checked_malloc(sizeof(*typed_node));
            typed_node->kind = ND_CONST_INT;
            typed_node->val_int = untyped_node->val_int;
            return typed_node;
        }

        case ND_LOCAL_VAR: {
            TypedNode typed_node = checked_malloc(sizeof(*typed_node));
            typed_node->kind = ND_LOCAL_VAR;
            return typed_node;
        }

        case ND_DEREF: {
            Vec children = new_vec_with_capacity(1);
            vec_push(children, check()) TypedNode typed_node =
                checked_malloc(sizeof(*typed_node));
            typed_node->kind = ND_DEREF;
            typed_node->children =
        };  // *[0]
        case ND_ADDR:
            break;  // &[0]
        case ND_MUL:
            break;  // [0] * [1]
        case ND_DIV:
            break;  // [0] / [1]
        case ND_ADD:
            break;  // [0] + [1]
        case ND_SUB:
            break;  // [0] - [1]
        case ND_LESS:
            break;  // [0] < [1]
        case ND_LESS_OR_EQUAL:
            break;  // [0] <= [1]
        case ND_EQUAL:
            break;  // [0] == [1]
        case ND_NOT_EQUAL:
            break;  // [0] != [1]
        case ND_ASSIGN:
            break;  // [0] = [1]
        case ND_RETURN:
            break;  // return [0]
        case ND_IF:
            break;  // if ([0]) [1]
        case ND_IF_ELSE:
            break;  // if ([0]) [1] else [2]
        case ND_WHILE:
            break;  // while ([0]) [1]
        case ND_FOR:
            break;  // for ([0]; [1]; [2]) [3]
        case ND_NULL:
            break;  // 空文
        case ND_CALL:
            break;  // 関数呼び出し func_name([0]:break; [1]:break; [2]:break;
                    // [3]:break; [4]:break; ...)
    }
}
