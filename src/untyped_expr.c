#include "compiler.h"

static int children_num(ExprKind kind) {
    switch (kind) {
        // 終端記号
        case EXPR_CONST_INT:
        case EXPR_LOCAL_VAR:
        case EXPR_GLOBAL_VAR:
        case EXPR_FUNC:
        case EXPR_STRING:
            return 0;

        // 非終端記号（子の個数が固定）
        case EXPR_ADDR:
        case EXPR_DEREF:
        case EXPR_SIZEOF:
        case EXPR_DECAY:
            return 1;
        case EXPR_MUL:
        case EXPR_DIV:
        case EXPR_MOD:
        case EXPR_ADD:
        case EXPR_SUB:
        case EXPR_LESS:
        case EXPR_LESS_OR_EQUAL:
        case EXPR_EQUAL:
        case EXPR_NOT_EQUAL:
        case EXPR_ASSIGN:
        case EXPR_COMPOUND_ADD:
            return 2;

            // 非終端記号（子の個数が可変）
            // case EXPR_BLOCK:
            // case EXPR_FUNC:
            // case EXPR_PROGRAM:
        case EXPR_CALL:
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
    untyped_expr->kind = EXPR_CONST_INT;
    untyped_expr->val_int = val_int;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var(Item item) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXPR_LOCAL_VAR;
    untyped_expr->item = item;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var_or_func_or_global_var(Scope scope,
                                                             char *name) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->item = scope_get_item(scope, name);
    switch (untyped_expr->item->kind) {
        case IT_FUNC:
            untyped_expr->kind = EXPR_FUNC;
            break;

        case IT_LOCAL_VAR:
            untyped_expr->kind = EXPR_LOCAL_VAR;
            break;

        case IT_GLOBAL_VAR:
            untyped_expr->kind = EXPR_GLOBAL_VAR;
            break;

        default:
            error("unimplemented: new_untyped_expr_identifier");
    }
    return untyped_expr;
}

UntypedExpr new_untyped_expr_call(Vec children) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXPR_CALL;
    untyped_expr->children = children;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_string(StringItem string_item) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXPR_STRING;
    untyped_expr->string_item = string_item;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var_with_def(Scope scope, Type type,
                                                char *name) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = EXPR_LOCAL_VAR;
    untyped_expr->item = scope_def_local_var(scope, type, name);
    return untyped_expr;
}

UntypedExpr untyped_expr_get_child(UntypedExpr untyped_expr, int index) {
    assert(index < untyped_expr->children->len);
    return untyped_expr->children->buf[index];
}
