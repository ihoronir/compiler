#include "compiler.h"

Stmt new_stmt_only_expr(UntypedExpr untyped_expr) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_ONLY_EXPR;
    stmt->untyped_expr = untyped_expr;
    return stmt;
}

Stmt new_stmt_return(UntypedExpr untyped_expr) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_RETURN;
    stmt->untyped_expr = untyped_expr;
    return stmt;
}

Stmt new_stmt_func_definition(Scope scope, Type type, char *name,
                              Vec untyped_expr_children, Vec stmt_children) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_FUNC_DEFINITION;
    stmt->untyped_expr_children = untyped_expr_children;
    stmt->stmt_children = stmt_children;
    stmt->item = scope_def_func(scope, type, name);
    return stmt;
}

Stmt new_stmt_block(Vec stmt_children);
Stmt new_stmt_block(Vec children) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_BLOCK;
    stmt->stmt_children = children;
    return stmt;
}

Stmt new_stmt_program(Vec stmt_children) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_PROGRAM;
    stmt->stmt_children = stmt_children;
    return stmt;
}

Stmt new_stmt_if(UntypedExpr cond, Stmt then_stmt) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_IF;

    stmt->untyped_expr_children = new_vec_with_capacity(1);
    vec_push(stmt->untyped_expr_children, cond);

    stmt->stmt_children = new_vec_with_capacity(1);
    vec_push(stmt->stmt_children, then_stmt);

    return stmt;
}

Stmt new_stmt_if_else(UntypedExpr cond, Stmt then_stmt, Stmt else_stmt) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_IF_ELSE;

    stmt->untyped_expr_children = new_vec_with_capacity(1);
    vec_push(stmt->untyped_expr_children, cond);

    stmt->stmt_children = new_vec_with_capacity(2);
    vec_push(stmt->stmt_children, then_stmt);
    vec_push(stmt->stmt_children, else_stmt);

    return stmt;
}

Stmt new_stmt_while(UntypedExpr cond, Stmt loop_stmt) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_WHILE;

    stmt->untyped_expr_children = new_vec_with_capacity(1);
    vec_push(stmt->untyped_expr_children, cond);

    stmt->stmt_children = new_vec_with_capacity(1);
    vec_push(stmt->stmt_children, loop_stmt);

    return stmt;
}

Stmt new_stmt_for(UntypedExpr init, UntypedExpr cond, UntypedExpr update,
                  Stmt loop_stmt) {
    Stmt stmt = checked_malloc(sizeof(*stmt));
    stmt->kind = STMT_FOR;

    stmt->untyped_expr_children = new_vec_with_capacity(3);
    vec_push(stmt->untyped_expr_children, init);
    vec_push(stmt->untyped_expr_children, cond);
    vec_push(stmt->untyped_expr_children, update);

    stmt->stmt_children = new_vec_with_capacity(1);
    vec_push(stmt->stmt_children, loop_stmt);

    return stmt;
}

Stmt stmt_get_stmt_child(Stmt stmt, int index) {
    assert(index < stmt->stmt_children->len);
    return stmt->stmt_children->buf[index];
}

UntypedExpr stmt_get_untyped_expr_child(Stmt stmt, int index) {
    assert(index < stmt->untyped_expr_children->len);
    return stmt->untyped_expr_children->buf[index];
}
