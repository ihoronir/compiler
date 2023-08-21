#include "compiler.h"

static int children_num(TypedExprKind kind) {
    switch (kind) {
        // 終端記号
        case TEXP_CONST_INT:
        case TEXP_LOCAL_VAR:
        case TEXP_GLOBAL_VAR:
        case TEXP_FUNC:
        case TEXP_STRING:
            return 0;

        // 非終端記号（子の個数が固定）
        case TEXP_ADDR:
        case TEXP_DEREF:
        case TEXP_DECAY:
            return 1;

        case TEXP_MUL:
        case TEXP_DIV:
        case TEXP_MOD:
        case TEXP_ADD:
        case TEXP_SUB:
        case TEXP_LESS:
        case TEXP_LESS_OR_EQUAL:
        case TEXP_SHL:
        case TEXP_SHR:
        case TEXP_SAR:
        case TEXP_EQUAL:
        case TEXP_NOT_EQUAL:
        case TEXP_ASSIGN:
        case TEXP_COMPOUND_ADD:
            return 2;

        case TEXP_CALL:
            return -1;
    }

    assert(0);
}

TypedExpr new_typed_expr_having_n_children(TypedExprKind kind, Type type,
                                           Vec children) {
    assert(children_num(kind) < 0);
    TypedExpr typed_expr = checked_malloc(sizeof(*typed_expr));
    typed_expr->kind = kind;
    typed_expr->children = children;
    typed_expr->type = type;
    return typed_expr;
}

TypedExpr new_typed_expr_having_2_children(TypedExprKind kind, Type type,
                                           TypedExpr child1, TypedExpr child2) {
    assert(children_num(kind) == 2);

    TypedExpr typed_expr = checked_malloc(sizeof(*typed_expr));
    typed_expr->kind = kind;
    typed_expr->children = new_vec_with_capacity(2);
    vec_push(typed_expr->children, child1);
    vec_push(typed_expr->children, child2);
    typed_expr->type = type;

    return typed_expr;
}

TypedExpr new_typed_expr_having_1_child(TypedExprKind kind, Type type,
                                        TypedExpr child) {
    assert(children_num(kind) == 1);

    TypedExpr typed_expr = checked_malloc(sizeof(*typed_expr));
    typed_expr->kind = kind;
    typed_expr->children = new_vec_with_capacity(1);
    vec_push(typed_expr->children, child);
    typed_expr->type = type;

    return typed_expr;
}

TypedExpr new_typed_expr_having_item(TypedExprKind kind, Item item) {
    assert(kind == TEXP_FUNC || kind == TEXP_LOCAL_VAR ||
           kind == TEXP_GLOBAL_VAR);

    TypedExpr typed_expr = checked_malloc(sizeof(*typed_expr));
    typed_expr->kind = kind;
    typed_expr->item = item;
    typed_expr->type = item->type;
    return typed_expr;
}

TypedExpr new_typed_expr_const_int(int val_int) {
    TypedExpr typed_expr = checked_malloc(sizeof(*typed_expr));
    typed_expr->kind = TEXP_CONST_INT;
    typed_expr->val_int = val_int;
    typed_expr->type = new_type_int();
    return typed_expr;
}

TypedExpr new_typed_expr_string(StringItem string_item) {
    TypedExpr typed_expr = checked_malloc(sizeof(*typed_expr));
    typed_expr->kind = TEXP_STRING;
    typed_expr->string_item = string_item;
    typed_expr->type =
        new_type_arr(new_type_char(), strlen(string_item->str) + 1);
    return typed_expr;
}

TypedExpr typed_expr_get_child(TypedExpr typed_expr, int index) {
    assert(children_num(typed_expr->kind) != 0);
    assert(index < typed_expr->children->len);
    return typed_expr->children->buf[index];
}
