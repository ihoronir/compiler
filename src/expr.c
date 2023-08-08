#include "compiler.h"

static int children_num(ExprKind kind) {
    switch (kind) {
        // 終端記号
        case EXP_CONST_INT:
        case EXP_LOCAL_VAR:
            return 0;

        // 非終端記号（子の個数が固定）
        case EXP_ADDR:
        case EXP_DEREF:
            return 1;
        case EXP_MUL:
        case EXP_DIV:
        case EXP_ADD:
        case EXP_SUB:
        case EXP_LESS:
        case EXP_LESS_OR_EQUAL:
        case EXP_EQUAL:
        case EXP_NOT_EQUAL:
        case EXP_ASSIGN:
            return 2;

            // 非終端記号（子の個数が可変）
            // case EXP_BLOCK:
            // case EXP_FUNC:
            // case EXP_PROGRAM:
        case EXP_CALL:
            return -1;
    }

    error("children_num: unreacnable");
    return 0;
}

UntypedExpr new_untyped_expr(ExprKind kind, ...) {
    int n = children_num(kind);
    assert(n > 0);

    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = kind;
    untyped_expr->children = new_vec_with_capacity(n);

    va_list ap;
    va_start(ap, kind);

    UntypedExpr untyped_expr_child;
    while ((untyped_expr_child = va_arg(ap, UntypedExpr)) != NULL) {
        vec_push(untyped_expr->children, untyped_expr_child);
    }

    va_end(ap);
    assert(untyped_expr->children->len == n);
    return untyped_expr;
}

UntypedExpr new_untyped_expr_const_int(int val_int) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXP_CONST_INT;
    untyped_expr->val_int = val_int;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var(Scope scope, char *name) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXP_LOCAL_VAR;
    untyped_expr->item = scope_get_item(IT_LOCAL_VAR, scope, name);
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var_with_def(Scope scope, Type type,
                                                char *name) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXP_LOCAL_VAR;
    untyped_expr->item = scope_def_local_var(scope, type, name);
    return untyped_expr;
}

UntypedExpr new_untyped_expr_call(Scope scope, char *name, Vec children) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXP_CALL;
    untyped_expr->children = children;
    untyped_expr->item = scope_get_item(IT_FUNC, scope, name);
    return untyped_expr;
}

UntypedExpr untyped_expr_get_child(UntypedExpr untyped_expr, int index) {
    if (index >= untyped_expr->children->len)
        error("untyped_expr_get_child: 範囲外アクセス");
    return untyped_expr->children->buf[index];
}