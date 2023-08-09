#include "compiler.h"

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
