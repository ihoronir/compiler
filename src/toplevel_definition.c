#include "compiler.h"

ToplevelDefinition new_toplevel_definition_func(Scope scope, Type type,
                                                char *name,
                                                Vec untyped_expr_children,
                                                Vec stmt_children) {
    ToplevelDefinition tld = checked_malloc(sizeof(*tld));
    tld->kind = TLD_FUNC_DEF;
    tld->untyped_expr_children = untyped_expr_children;
    tld->stmt_children = stmt_children;
    tld->item = scope_def_func(scope, type, name);
    return tld;
}
