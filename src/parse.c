#include "compiler.h"

static int consume(TokenKind tk) {
    Token token = tokens_peek();
    if (token->kind != tk) return 0;
    tokens_next();
    return 1;
}

static char *consume_ident() {
    Token token = tokens_peek();
    if (token->kind != TK_IDENT) return NULL;
    tokens_next();
    return token->str;
}

static void expect(TokenKind tk) {
    Token token = tokens_peek();
    if (token->kind != tk) {
        fprintf(stderr, "expected: %d, actuall: %d", tk, token->kind);
        error_at(token->line, token->row, "期待される字句ではありません");
    }
    tokens_next();
}

static char *expect_ident() {
    Token token = tokens_peek();
    if (token->kind != TK_IDENT)
        error_at(token->line, token->row, "識別子ではありません");
    tokens_next();
    return token->str;
}

static int expect_const_int() {
    Token token = tokens_peek();
    if (token->kind != TK_CONST_INT)
        error_at(token->line, token->row, "数ではありません");
    tokens_next();
    return token->val_int;
}

static UntypedExpr parse_expr(Scope scope);

// primary = "(" expr ")"
//         | ident
//         | num
static UntypedExpr parse_primary(Scope scope) {
    if (consume(TK_LEFT_PAREN)) {
        UntypedExpr untyped_node = parse_expr(scope);
        expect(TK_RIGHT_PAREN);
        return untyped_node;
    }

    char *name;
    if ((name = consume_ident()) != NULL) {
        return new_untyped_expr_local_var_or_func(scope, name);
    }

    // そうでなければ数値のはず
    UntypedExpr untyped_node = new_untyped_expr_const_int(expect_const_int());
    return untyped_node;
}

// postfix = primary ( "[" expr "]" |  "(" expr, expr, ... ")" )*
static UntypedExpr parse_postfix(Scope scope) {
    UntypedExpr untyped_expr = parse_primary(scope);

    for (;;) {
        if (consume(TK_LEFT_SQ_BRACKET)) {
            UntypedExpr index = parse_expr(scope);
            expect(TK_RIGHT_SQ_BRACKET);

            UntypedExpr add =
                new_untyped_expr(EXP_ADD, untyped_expr, index, NULL);

            UntypedExpr deref = new_untyped_expr(EXP_DEREF, add, NULL);

            untyped_expr = deref;

        } else if (consume(TK_LEFT_PAREN)) {
            Vec children = new_vec();
            vec_push(children, untyped_expr);

            if (!consume(TK_RIGHT_PAREN)) {
                do {
                    vec_push(children, parse_expr(scope));
                } while (consume(TK_COMMA));

                expect(TK_RIGHT_PAREN);
            }

            untyped_expr = new_untyped_expr_call(children);

        } else {
            return untyped_expr;
        }
    }
}

// unary = "sizeof" unary
//       | ("+" | "-" | "*" | "&" )? unary
//       | postfix
static UntypedExpr parse_unary(Scope scope) {
    if (consume(TK_SIZEOF)) {
        return new_untyped_expr(EXP_SIZEOF, parse_unary(scope), NULL);

    } else if (consume(TK_PLUS)) {
        return parse_unary(scope);

    } else if (consume(TK_MINUS)) {
        return new_untyped_expr(EXP_SUB, new_untyped_expr_const_int(0),
                                parse_unary(scope), NULL);

    } else if (consume(TK_ASTERISK)) {
        return new_untyped_expr(EXP_DEREF, parse_unary(scope), NULL);

    } else if (consume(TK_AND)) {
        return new_untyped_expr(EXP_ADDR, parse_unary(scope), NULL);
    }

    return parse_postfix(scope);
}

// mul = unary ("*" unary | "/" unary)*
static UntypedExpr parse_mul(Scope scope) {
    UntypedExpr untyped_node = parse_unary(scope);

    for (;;) {
        if (consume(TK_ASTERISK)) {
            untyped_node = new_untyped_expr(EXP_MUL, untyped_node,
                                            parse_unary(scope), NULL);

        } else if (consume(TK_SLASH)) {
            untyped_node = new_untyped_expr(EXP_DIV, untyped_node,
                                            parse_unary(scope), NULL);

        } else {
            return untyped_node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
static UntypedExpr parse_add(Scope scope) {
    UntypedExpr untyped_node = parse_mul(scope);

    for (;;) {
        if (consume(TK_PLUS)) {
            untyped_node =
                new_untyped_expr(EXP_ADD, untyped_node, parse_mul(scope), NULL);

        } else if (consume(TK_MINUS)) {
            untyped_node =
                new_untyped_expr(EXP_SUB, untyped_node, parse_mul(scope), NULL);

        } else {
            return untyped_node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static UntypedExpr parse_relational(Scope scope) {
    UntypedExpr untyped_node = parse_add(scope);

    for (;;) {
        if (consume(TK_LESS)) {
            untyped_node = new_untyped_expr(EXP_LESS, untyped_node,
                                            parse_add(scope), NULL);

        } else if (consume(TK_LESS_EQUAL)) {
            untyped_node = new_untyped_expr(EXP_LESS_OR_EQUAL, untyped_node,
                                            parse_add(scope), NULL);

        } else if (consume(TK_MORE)) {
            untyped_node = new_untyped_expr(EXP_LESS, parse_add(scope),
                                            untyped_node, NULL);

        } else if (consume(TK_MORE_EQUAL)) {
            untyped_node = new_untyped_expr(EXP_LESS_OR_EQUAL, parse_add(scope),
                                            untyped_node, NULL);

        } else {
            return untyped_node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
static UntypedExpr parse_equality(Scope scope) {
    UntypedExpr untyped_node = parse_relational(scope);

    for (;;) {
        if (consume(TK_EQUAL_EQUAL)) {
            untyped_node = new_untyped_expr(EXP_EQUAL, untyped_node,
                                            parse_relational(scope), NULL);

        } else if (consume(TK_EXCL_EQUAL)) {
            untyped_node = new_untyped_expr(EXP_NOT_EQUAL, untyped_node,
                                            parse_relational(scope), NULL);

        } else {
            return untyped_node;
        }
    }
}
// assign = equality ("=" assign)?
static UntypedExpr parse_assign(Scope scope) {
    UntypedExpr untyped_node = parse_equality(scope);

    if (consume(TK_EQUAL)) {
        untyped_node = new_untyped_expr(EXP_ASSIGN, untyped_node,
                                        parse_assign(scope), NULL);
    }

    return untyped_node;
}

// expr = assign
static UntypedExpr parse_expr(Scope scope) { return parse_assign(scope); }

// stmt = "int" ( "*" )* ident ("[" const_int "]")? ";")
//      | ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
//      | "int" ident ";"
//      | expr ";"
static Stmt parse_stmt(Scope scope) {
    // "int" ident ";"
    if (consume(TK_INT)) {
        Type type = new_type_int();

        for (;;) {
            if (consume(TK_ASTERISK)) {
                type = new_type_ptr(type);

            } else {
                char *name = expect_ident();

                if (consume(TK_LEFT_SQ_BRACKET)) {
                    int arr_len = expect_const_int();
                    type = new_type_arr(type, arr_len);
                    expect(TK_RIGHT_SQ_BRACKET);
                }
                scope_def_local_var(scope, type, name);
                break;
            }
        }
        expect(TK_SEMICOLON);
        return NULL;
    }

    // ";"
    if (consume(TK_SEMICOLON)) return NULL;

    // "{" stmt* "}"
    if (consume(TK_LEFT_BRACE)) {
        Vec children = new_vec();

        Scope block_scope = new_scope(scope);
        while (!consume(TK_RIGHT_BRACE)) {
            Stmt stmt_child = parse_stmt(block_scope);
            if (stmt_child != NULL) {
                vec_push(children, stmt_child);
            }
        }

        return new_stmt_block(children);
    }

    // "if" "(" expr ")" stmt ("else" stmt)?
    if (consume(TK_IF)) {
        expect(TK_LEFT_PAREN);
        UntypedExpr cond = parse_expr(scope);
        expect(TK_RIGHT_PAREN);
        Stmt then = parse_stmt(scope);

        if (consume(TK_ELSE)) {
            return new_stmt_if_else(cond, then, parse_stmt(scope));

        } else {
            return new_stmt_if(cond, then);
        }
    }

    // "while" "(" expr ")" stmt
    if (consume(TK_WHILE)) {
        expect(TK_LEFT_PAREN);
        UntypedExpr cond = parse_expr(scope);
        expect(TK_RIGHT_PAREN);

        return new_stmt_while(cond, parse_stmt(scope));
    }

    // "for" "(" expr? ";" expr? ";" expr? ")" stmt
    if (consume(TK_FOR)) {
        expect(TK_LEFT_PAREN);

        UntypedExpr init;
        if (consume(TK_SEMICOLON)) {
            init = NULL;

        } else {
            init = parse_expr(scope);
            expect(TK_SEMICOLON);
        }

        UntypedExpr cond;
        if (consume(TK_SEMICOLON)) {
            cond = new_untyped_expr_const_int(1);

        } else {
            cond = parse_expr(scope);
            expect(TK_SEMICOLON);
        }

        UntypedExpr update;
        if (consume(TK_RIGHT_PAREN)) {
            update = NULL;

        } else {
            update = parse_expr(scope);
            expect(TK_RIGHT_PAREN);
        }

        return new_stmt_for(init, cond, update, parse_stmt(scope));
    }

    // "return" expr ";"
    if (consume(TK_RETURN)) {
        Stmt stmt = new_stmt_return(parse_expr(scope));
        expect(TK_SEMICOLON);
        return stmt;
    }

    // expr ";"
    Stmt stmt = new_stmt_only_expr(parse_expr(scope));
    expect(TK_SEMICOLON);
    return stmt;
}

// func = "int" ident "(" ("int" ident ",")* ")" "{" stmt* "}"
static ToplevelDefinition parse_toplevel_definition(Scope scope) {
    expect(TK_INT);

    Type type = new_type_int();
    Vec untyped_expr_children = new_vec();
    Vec stmt_children = new_vec();
    ToplevelDefinition tld;

    for (;;) {
        if (consume(TK_ASTERISK)) {
            type = new_type_ptr(type);

        } else {
            break;
        }
    }

    tld =
        new_toplevel_definition_func(scope, new_type_func(type), expect_ident(),
                                     untyped_expr_children, stmt_children);

    Scope func_scope = new_scope_func(scope, tld->item);

    expect(TK_LEFT_PAREN);
    if (!consume(TK_RIGHT_PAREN)) {
        do {
            expect(TK_INT);

            UntypedExpr untyped_expr;
            Type type = new_type_int();

            for (;;) {
                if (consume(TK_ASTERISK)) {
                    type = new_type_ptr(type);

                } else {
                    untyped_expr = new_untyped_expr_local_var_with_def(
                        func_scope, type, expect_ident());
                    break;
                }
            }

            vec_push(untyped_expr_children, untyped_expr);

        } while (consume(TK_COMMA));
        expect(TK_RIGHT_PAREN);
    }

    if (consume(TK_LEFT_BRACE)) {
        Vec func_block_children = new_vec();

        while (!consume(TK_RIGHT_BRACE)) {
            Stmt func_block_child = parse_stmt(func_scope);
            if (func_block_child != NULL) {
                vec_push(func_block_children, func_block_child);
            }
        }

        vec_push(stmt_children, new_stmt_block(func_block_children));

        return tld;

    } else {
        expect(TK_SEMICOLON);
        return NULL;
    }
}

// program = func*;
Vec /* <ToplevelDefinition> */ parse_program() {
    Vec children = new_vec();
    Scope scope = new_scope_global();

    while (!consume(TK_EOF)) {
        ToplevelDefinition fn = parse_toplevel_definition(scope);
        if (fn != NULL) vec_push(children, fn);
    }

    return children;
}
