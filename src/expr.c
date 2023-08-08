#include "compiler.h"

TypedExpr to_typed(UntypedExpr ue) {
    TypedExpr te = checked_malloc(sizeof(*te));
    te->kind = ue->kind;
    te->item = ue->item;

    switch (ue->kind) {
        case EXP_CONST_INT:
            te->val_int = ue->val_int;
            te->type = new_type_int();
            return te;

        case EXP_LOCAL_VAR:
            te->type = ue->item->type;
            return te;

        case EXP_DEREF: {
            UntypedExpr u_child = untyped_expr_get_child(ue, 0);
            TypedExpr t_child = to_typed(u_child);
            if (t_child->type->kind != TY_PTR)
                error("ポインタではないものを Deref しました");

            te->type = t_child->type->ptr_to;
            te->children = new_vec_with_capacity(1);
            vec_push(te->children, t_child);
            return te;
        }

        case EXP_ADDR: {
            UntypedExpr u_child = untyped_expr_get_child(ue, 0);
            TypedExpr t_child = to_typed(u_child);

            te->type = new_type_ptr(t_child->type);
            te->children = new_vec_with_capacity(1);
            vec_push(te->children, t_child);
            return te;
        }

        case EXP_DIV:
        case EXP_MUL: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = to_typed(u_lhs);

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = to_typed(u_rhs);

            if (t_lhs->type->kind != TY_INT || t_rhs->type->kind != TY_INT)
                error("INT 同士でしか掛け算・割り算はできません");

            te->type = new_type_int();
            te->children = new_vec_with_capacity(2);
            vec_push(te->children, t_lhs);
            vec_push(te->children, t_rhs);
            return te;
        }

        case EXP_ADD: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = to_typed(u_lhs);

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = to_typed(u_rhs);

            if (t_lhs->type->kind == TY_INT && t_rhs->type->kind == TY_INT) {
                te->type = new_type_int();

                te->children = new_vec_with_capacity(2);
                vec_push(te->children, t_lhs);
                vec_push(te->children, t_rhs);

            } else if (t_lhs->type->kind == TY_PTR &&
                       t_rhs->type->kind == TY_INT) {
                te->type = t_lhs->type;

                te->children = new_vec_with_capacity(2);
                vec_push(te->children, t_lhs);
                int size = type_size(t_lhs->type->ptr_to);
                TypedExpr multiplied = to_typed(new_untyped_expr(
                    EXP_MUL, u_rhs, new_untyped_expr_const_int(size), NULL));
                vec_push(te->children, multiplied);

            } else if (t_lhs->type->kind == TY_INT &&
                       t_rhs->type->kind == TY_PTR) {
                te->type = t_rhs->type;

                te->children = new_vec_with_capacity(2);
                int size = type_size(t_rhs->type->ptr_to);
                TypedExpr multiplied = to_typed(new_untyped_expr(
                    EXP_MUL, u_lhs, new_untyped_expr_const_int(size), NULL));
                vec_push(te->children, multiplied);
                vec_push(te->children, t_rhs);
            } else {
                error("ポインタとポインタの加算はできません");
            }

            return te;
        }

        case EXP_SUB: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = to_typed(u_lhs);

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = to_typed(u_rhs);

            if (t_lhs->type->kind == TY_INT && t_rhs->type->kind == TY_INT) {
                return te;

            } else {
                error("整数同士以外の引き算は未実装です");
            }
        }

        case EXP_LESS_OR_EQUAL:
        case EXP_EQUAL:
        case EXP_NOT_EQUAL:
        case EXP_LESS: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = to_typed(u_lhs);

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = to_typed(u_rhs);

            if (!type_is_equal(t_lhs->type, t_rhs->type))
                error("異なる型同士の大小は比較できません");

            te->type = new_type_int();
            te->children = new_vec_with_capacity(2);
            vec_push(te->children, t_lhs);
            vec_push(te->children, t_rhs);
            return te;
        }

        case EXP_ASSIGN: {
            UntypedExpr u_lhs = untyped_expr_get_child(ue, 0);
            TypedExpr t_lhs = to_typed(u_lhs);

            UntypedExpr u_rhs = untyped_expr_get_child(ue, 1);
            TypedExpr t_rhs = to_typed(u_rhs);

            if (!type_is_equal(t_lhs->type, t_rhs->type))
                error("異なる型同士の大小は比較できません");

            te->type = t_lhs->type;
            te->children = new_vec_with_capacity(2);
            vec_push(te->children, t_lhs);
            vec_push(te->children, t_rhs);
            return te;
        }

        case EXP_CALL: {
            te->type = ue->item->type;

            Vec te_children = new_vec();

            for (int i = 0; i < ue->children->len; i++) {
                vec_push(te_children, to_typed(untyped_expr_get_child(ue, i)));
            }
            te->children = te_children;

            return te;
        }
    }
}
