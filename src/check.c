#include "compiler.h"

// te->type が TY_ARR ならば、その te を子供に持つ EXPR_DECAY
// を作る その EXPR_DECAY は TY_PTR である
TypedExpr decay_if_array(TypedExpr typed_expr) {
    if (typed_expr->type->kind != TY_ARR) return typed_expr;

    return new_typed_expr_having_1_child(
        TEXP_DECAY, new_type_ptr(typed_expr->type->ptr_to), typed_expr);
}

TypedExpr to_typed_expr(UntypedExpr untyped_expr) {
    // TypedExpr te = checked_malloc(sizeof(*te));
    // te->kind = ue->kind;
    // te->item = ue->item;

    switch (untyped_expr->kind) {
        case UEXP_SIZEOF: {
            UntypedExpr untyped_child = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_child = to_typed_expr(untyped_child);
            return new_typed_expr_const_int(type_size(typed_child->type));
        }

        case UEXP_CONST_INT:
            return new_typed_expr_const_int(untyped_expr->val_int);

        case UEXP_STRING:
            return new_typed_expr_string(untyped_expr->string_item);

        // TODO UEXP_ITEM ? とかに統一
        case UEXP_FUNC:
            return new_typed_expr_having_item(TEXP_FUNC, untyped_expr->item);

        case UEXP_LOCAL_VAR:
            return new_typed_expr_having_item(TEXP_LOCAL_VAR,
                                              untyped_expr->item);

        case UEXP_GLOBAL_VAR:
            return new_typed_expr_having_item(TEXP_GLOBAL_VAR,
                                              untyped_expr->item);

        case UEXP_DEREF: {
            UntypedExpr untyped_child = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_child =
                decay_if_array(to_typed_expr(untyped_child));

            if (!type_is_pointer(typed_child->type))
                error("ポインタではないものを Deref しました");

            return new_typed_expr_having_1_child(
                TEXP_DEREF, typed_child->type->ptr_to, typed_child);
        }

        case UEXP_ADDR: {
            UntypedExpr untyped_child = untyped_expr_get_child(untyped_expr, 0);
            // decay_if_array しない！！
            TypedExpr typed_child = to_typed_expr(untyped_child);

            return new_typed_expr_having_1_child(
                TEXP_ADDR, new_type_ptr(typed_child->type), typed_child);
        }

        case UEXP_MUL: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (typed_lhs->type->kind != TY_INT ||
                typed_rhs->type->kind != TY_INT)
                error("* は整数同士でしかつかえません");

            return new_typed_expr_having_2_children(TEXP_MUL, new_type_int(),
                                                    typed_lhs, typed_rhs);
        }

        case UEXP_DIV: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (typed_lhs->type->kind != TY_INT ||
                typed_rhs->type->kind != TY_INT)
                error("/ は整数同士でしかつかえません");

            return new_typed_expr_having_2_children(TEXP_DIV, new_type_int(),
                                                    typed_lhs, typed_rhs);
        }

        case UEXP_MOD: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (typed_lhs->type->kind != TY_INT ||
                typed_rhs->type->kind != TY_INT)
                error("% は整数同士でしかつかえません");

            return new_typed_expr_having_2_children(TEXP_MOD, new_type_int(),
                                                    typed_lhs, typed_rhs);
        }

        case UEXP_ADD: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (type_is_integer(typed_lhs->type) &&
                type_is_integer(typed_rhs->type)) {
                return new_typed_expr_having_2_children(
                    TEXP_ADD, new_type_int(), typed_lhs, typed_rhs);
            }

            if (type_is_pointer(typed_lhs->type) &&
                type_is_integer(typed_rhs->type)) {
                TypedExpr stride = new_typed_expr_const_int(
                    type_size(typed_lhs->type->ptr_to));
                // multiplied の結果は ptr にするかも？
                TypedExpr multiplied = new_typed_expr_having_2_children(
                    TEXP_MUL, new_type_int(), typed_rhs, stride);
                return new_typed_expr_having_2_children(
                    TEXP_ADD, typed_lhs->type, typed_lhs, multiplied);
            }

            if (type_is_integer(typed_lhs->type) &&
                type_is_pointer(typed_rhs->type)) {
                TypedExpr stride = new_typed_expr_const_int(
                    type_size(typed_rhs->type->ptr_to));
                // multiplied の結果は ptr にするかも？
                TypedExpr multiplied = new_typed_expr_having_2_children(
                    TEXP_MUL, new_type_int(), typed_lhs, stride);
                return new_typed_expr_having_2_children(
                    TEXP_ADD, typed_rhs->type, typed_rhs, multiplied);
            }

            error("ポインタとポインタの加算はできません");
        }

        case UEXP_SUB: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (type_is_integer(typed_lhs->type) &&
                type_is_integer(typed_rhs->type)) {
                return new_typed_expr_having_2_children(
                    TEXP_SUB, new_type_int(), typed_lhs, typed_rhs);
            }

            error("整数同士以外の引き算は未実装です");
        }

        case UEXP_LESS_OR_EQUAL: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (!type_is_equal(typed_lhs->type, typed_rhs->type))
                error("異なる型同士の大小は比較できません");

            return new_typed_expr_having_2_children(
                TEXP_LESS_OR_EQUAL, new_type_int(), typed_lhs, typed_rhs);
        }

        case UEXP_LESS: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (!type_is_equal(typed_lhs->type, typed_rhs->type))
                error("異なる型同士の大小は比較できません");

            return new_typed_expr_having_2_children(TEXP_LESS, new_type_int(),
                                                    typed_lhs, typed_rhs);
        }

        case UEXP_EQUAL: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (!type_is_equal(typed_lhs->type, typed_rhs->type))
                error("異なる型同士は==比較できません");

            return new_typed_expr_having_2_children(TEXP_EQUAL, new_type_int(),
                                                    typed_lhs, typed_rhs);
        }

        case UEXP_NOT_EQUAL: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = decay_if_array(to_typed_expr(untyped_lhs));

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (!type_is_equal(typed_lhs->type, typed_rhs->type))
                error("異なる型同士は!=比較できません");

            return new_typed_expr_having_2_children(
                TEXP_NOT_EQUAL, new_type_int(), typed_lhs, typed_rhs);
        }

        case UEXP_ASSIGN: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = to_typed_expr(untyped_lhs);
            if (typed_lhs->type->kind == TY_ARR)
                error("配列型には直接代入できません");

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (!type_is_compatible(typed_lhs->type, typed_rhs->type))
                error(
                    "対応していない型同士の変数には代入でき"
                    "せん");

            return new_typed_expr_having_2_children(
                TEXP_ASSIGN, typed_lhs->type, typed_lhs, typed_rhs);
        }

        case UEXP_COMPOUND_ADD: {
            UntypedExpr untyped_lhs = untyped_expr_get_child(untyped_expr, 0);
            TypedExpr typed_lhs = to_typed_expr(untyped_lhs);

            if (typed_lhs->type->kind == TY_ARR)
                error("配列型は += の左辺にこれません");

            UntypedExpr untyped_rhs = untyped_expr_get_child(untyped_expr, 1);
            TypedExpr typed_rhs = decay_if_array(to_typed_expr(untyped_rhs));

            if (!type_is_integer(typed_rhs->type))
                error("+= の右辺は整数型のみとれます");

            return new_typed_expr_having_2_children(
                TEXP_COMPOUND_ADD, typed_lhs->type, typed_lhs, typed_rhs);
        }

        case UEXP_CALL: {
            UntypedExpr func = untyped_expr_get_child(untyped_expr, 0);
            if (func->item->type->kind != TY_FUNC)
                error("関数以外は呼べません");

            Vec typed_children = new_vec();
            vec_push(typed_children, to_typed_expr(func));

            for (int i = 1; i < untyped_expr->children->len; i++) {
                vec_push(typed_children,
                         decay_if_array(to_typed_expr(
                             untyped_expr_get_child(untyped_expr, i))));
            }

            return new_typed_expr_having_n_children(
                TEXP_CALL, func->item->type->returning, typed_children);
        }
    }

    assert(0);
}
void make_stmt_typed(Stmt stmt) {
    stmt->typed_expr_children = new_vec();
    for (int i = 0; i < stmt->untyped_expr_children->len; i++) {
        UntypedExpr untyped_child = stmt_get_untyped_expr_child(stmt, i);
        vec_push(stmt->typed_expr_children, to_typed_expr(untyped_child));
    }

    for (int i = 0; i < stmt->stmt_children->len; i++) {
        Stmt stmt_child = stmt_get_stmt_child(stmt, i);
        make_stmt_typed(stmt_child);
    }
}
