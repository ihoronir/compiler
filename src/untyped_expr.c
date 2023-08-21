#include <assert.h>

#include "compiler.h"

static int children_num(UntypedExprKind kind) {
    switch (kind) {
        // 終端記号
        case UEXP_CONST_INT:
        case UEXP_LOCAL_VAR:
        case UEXP_GLOBAL_VAR:
        case UEXP_FUNC:
        case UEXP_STRING:
            return 0;

        // 非終端記号（子の個数が固定）
        case UEXP_ADDR:
        case UEXP_DEREF:
        case UEXP_SIZEOF:
            return 1;
        case UEXP_MUL:
        case UEXP_DIV:
        case UEXP_MOD:
        case UEXP_ADD:
        case UEXP_SUB:
        case UEXP_LESS:
        case UEXP_LESS_OR_EQUAL:
        case UEXP_SHL:
        case UEXP_SHR:
        case UEXP_EQUAL:
        case UEXP_NOT_EQUAL:
        case UEXP_ASSIGN:
        case UEXP_COMPOUND_ADD:
            return 2;

        case UEXP_CALL:
            return -1;
    }

    assert(0);
}

UntypedExpr new_untyped_expr(UntypedExprKind kind, ...) {
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
    untyped_expr->kind = UEXP_CONST_INT;
    untyped_expr->val_int = val_int;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var(Item item) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = UEXP_LOCAL_VAR;
    untyped_expr->item = item;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var_or_func_or_global_var(Scope scope,
                                                             char *name) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->item = scope_get_item(scope, name);
    switch (untyped_expr->item->kind) {
        case IT_FUNC:
            untyped_expr->kind = UEXP_FUNC;
            break;

        case IT_LOCAL_VAR:
            untyped_expr->kind = UEXP_LOCAL_VAR;
            break;

        case IT_GLOBAL_VAR:
            untyped_expr->kind = UEXP_GLOBAL_VAR;
            break;

        default:
            error("unimplemented: new_untyped_expr_identifier");
    }
    return untyped_expr;
}

UntypedExpr new_untyped_expr_call(Vec children) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = UEXP_CALL;
    untyped_expr->children = children;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_string(StringItem string_item) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = UEXP_STRING;
    untyped_expr->string_item = string_item;
    return untyped_expr;
}

UntypedExpr new_untyped_expr_local_var_with_def(Scope scope, Type type,
                                                char *name) {
    UntypedExpr untyped_expr = checked_malloc(sizeof(*untyped_expr));
    untyped_expr->kind = UEXP_LOCAL_VAR;
    untyped_expr->item = scope_def_local_var(scope, type, name);
    return untyped_expr;
}

UntypedExpr untyped_expr_get_child(UntypedExpr untyped_expr, int index) {
    assert(children_num(untyped_expr->kind) != 0);
    assert(index < untyped_expr->children->len);
    return untyped_expr->children->buf[index];
}
