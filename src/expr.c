#include "compiler.h"

// te->type が TY_ARR ならば、その te を子供に持つ EXPR_DECAY
// を作る その EXPR_DECAY は TY_PTR である
TypedExpr decay_if_array(TypedExpr te) {
    if (te->type->kind != TY_ARR) return te;

    TypedExpr new_te = checked_malloc(sizeof(*te));
    new_te->children = new_vec_with_capacity(1);
    vec_push(new_te->children, te);
    new_te->type = new_type_ptr(te->type->ptr_to);
    new_te->kind = EXPR_DECAY;
    return new_te;
}

TypedExpr to_typed_expr(UntypedExpr ue) {
    TypedExpr te = checked_malloc(sizeof(*te));
    te->kind = ue->kind;
    te->item = ue->item;

    switch (ue->kind) {
        case EXPR_DECAY:
            assert(0);

        case EXPR_SIZEOF: {
            te->kind = EXPR_CONST_INT;

            UntypedExpr u_child = untyped_expr_get_child(ue, 0);
            TypedExpr t_child = to_typed_expr(u_child);

            te->val_int = type_size(t_child->type);
            te->type = new_type_int();
            return te;
        }

        case EXPR_CONST_INT:
            te->val_int = ue->val_int;
            te->type = new_type_int();
            return te;

        case EXPR_STRING:
            te->type =
                new_type_arr(new_type_char(), strlen(ue->string_item->str) + 1);
            te->string_item = ue->string_item;
            return te;

        case EXPR_FUNC:
        case EXPR_LOCAL_VAR:
        case EXPR_GLOBAL_VAR:
            te->type = ue->item->type;
            return te;

        case EXPR_DEREF: {
            UntypedExpr u_child = untyped_expr_get_child(ue, 0);
            TypedExpr t_child = decay_if_array(to_typed_expr(u_child));
            if (t_child->type->kind != TY_PTR)
                error("ポインタではないものを Deref しました");

            te->type = t_child->type->ptr_to;
            te->children = new_vec_with_capacity(1);
            vec_push(te->children, t_child);
            return te;
        }

        case EXPR_ADDR: {
            UntypedExpr u_child = untyped_expr_get_child(ue, 0);
            // decay_if_array しない！！
            TypedExpr t_child = to_typed_expr(u_child);

            te->type = new_type_ptr(t_child->type);
            te->children = new_vec_with_capacity(1);
            vec_push(te->children, t_child);
            return te;
        }

        case EXPR_DIV:
        case EXPR_MUL:
        case EXPR_MOD: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = decay_if_array(to_typed_expr(u_lhs));

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = decay_if_array(to_typed_expr(u_rhs));

            if (t_lhs->type->kind != TY_INT || t_rhs->type->kind != TY_INT)
                error("*, /, % は整数同士でしかつかえません");

            te->type = new_type_int();
            te->children = new_vec_with_capacity(2);
            vec_push(te->children, t_lhs);
            vec_push(te->children, t_rhs);
            return te;
        }

        case EXPR_ADD: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = decay_if_array(to_typed_expr(u_lhs));

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = decay_if_array(to_typed_expr(u_rhs));

            if (type_is_integer(t_lhs->type) && type_is_integer(t_rhs->type)) {
                te->type = new_type_int();

                te->children = new_vec_with_capacity(2);
                vec_push(te->children, t_lhs);
                vec_push(te->children, t_rhs);

            } else if (t_lhs->type->kind == TY_PTR &&
                       type_is_integer(t_rhs->type)) {
                te->type = t_lhs->type;

                te->children = new_vec_with_capacity(2);
                vec_push(te->children, t_lhs);
                int size = type_size(t_lhs->type->ptr_to);
                TypedExpr multiplied = to_typed_expr(new_untyped_expr(
                    EXPR_MUL, u_rhs, new_untyped_expr_const_int(size), NULL));
                vec_push(te->children, multiplied);

            } else if (type_is_integer(t_lhs->type) &&
                       t_rhs->type->kind == TY_PTR) {
                te->type = t_rhs->type;

                te->children = new_vec_with_capacity(2);
                int size = type_size(t_rhs->type->ptr_to);
                TypedExpr multiplied = to_typed_expr(new_untyped_expr(
                    EXPR_MUL, u_lhs, new_untyped_expr_const_int(size), NULL));
                vec_push(te->children, multiplied);
                vec_push(te->children, t_rhs);
            } else {
                error("ポインタとポインタの加算はできません");
            }

            return te;
        }

        case EXPR_SUB: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = decay_if_array(to_typed_expr(u_lhs));

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = decay_if_array(to_typed_expr(u_rhs));

            if (t_lhs->type->kind == TY_INT && t_rhs->type->kind == TY_INT) {
                te->type = new_type_int();

                te->children = new_vec_with_capacity(2);
                vec_push(te->children, t_lhs);
                vec_push(te->children, t_rhs);
                return te;

            } else {
                error("整数同士以外の引き算は未実装です");
            }
        }

        case EXPR_LESS_OR_EQUAL:
        case EXPR_EQUAL:
        case EXPR_NOT_EQUAL:
        case EXPR_LESS: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = decay_if_array(to_typed_expr(u_lhs));

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = decay_if_array(to_typed_expr(u_rhs));

            if (!type_is_equal(t_lhs->type, t_rhs->type))
                error("異なる型同士の大小は比較できません");

            te->type = new_type_int();
            te->children = new_vec_with_capacity(2);
            vec_push(te->children, t_lhs);
            vec_push(te->children, t_rhs);
            return te;
        }

        case EXPR_ASSIGN: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = to_typed_expr(u_lhs);
            if (t_lhs->type->kind == TY_ARR)
                error("配列型には直接代入できません");

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = decay_if_array(to_typed_expr(u_rhs));

            if (!type_is_compatible(t_lhs->type, t_rhs->type))
                error(
                    "対応していない型同士の変数には代入でき?"
                    "せん");

            te->type = t_lhs->type;
            te->children = new_vec_with_capacity(2);
            vec_push(te->children, t_lhs);
            vec_push(te->children, t_rhs);
            return te;
        }

        case EXPR_COMPOUND_ADD: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = to_typed_expr(u_lhs);
            if (t_lhs->type->kind == TY_ARR)
                error("配列型は += の左辺にこれません");

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = decay_if_array(to_typed_expr(u_rhs));

            if (!type_is_integer(t_rhs->type))
                error("+= の右辺は整数型のみとれます");

            te->type = t_lhs->type;
            te->children = new_vec_with_capacity(2);
            vec_push(te->children, t_lhs);
            vec_push(te->children, t_rhs);
            return te;
        }

        case EXPR_CALL: {
            UntypedExpr func = untyped_expr_get_child(ue, 0);
            if (func->item->type->kind != TY_FUNC)
                error("関数以外は呼べません");

            te->type = func->item->type->returning;

            Vec te_children = new_vec();

            vec_push(te_children, to_typed_expr(func));

            for (int i = 1; i < ue->children->len; i++) {
                vec_push(te_children, decay_if_array(to_typed_expr(
                                          untyped_expr_get_child(ue, i))));
            }
            te->children = te_children;

            return te;
        }
    }

    assert(0);
}

TypedExpr typed_expr_get_child(TypedExpr typed_expr, int index) {
    assert(index < typed_expr->children->len);
    return typed_expr->children->buf[index];
}
