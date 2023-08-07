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
    if (token->kind != tk)
        error_at(token->line, token->row, "期待される字句ではありません");
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

static UntypedNode expr(Scope scope);

// primary = num
//         | ident ("(" ")")?
//         | "(" expr ")"
static UntypedNode primary(Scope scope) {
    if (consume(TK_LEFT_PAREN)) {
        UntypedNode untyped_node = expr(scope);
        expect(TK_RIGHT_PAREN);
        return untyped_node;
    }

    char *name;
    if ((name = consume_ident()) != NULL) {
        if (consume(TK_LEFT_PAREN)) {
            Vec children = new_vec();

            if (!consume(TK_RIGHT_PAREN)) {
                do {
                    vec_push(children, expr(scope));
                } while (consume(TK_COMMA));
                expect(TK_RIGHT_PAREN);
            }

            return new_untyped_node_call(scope, name, children);
        }

        return new_untyped_node_local_var(scope, name);
    }

    // そうでなければ数値のはず
    UntypedNode untyped_node = new_untyped_node_const_int(expect_const_int());
    return untyped_node;
}

// unary = ("+" | "-")? primary
static UntypedNode unary(Scope scope) {
    for (;;) {
        if (consume(TK_PLUS)) {
            return primary(scope);

        } else if (consume(TK_MINUS)) {
            return new_untyped_node(ND_SUB, new_untyped_node_const_int(0),
                                    primary(scope), NULL);

        } else if (consume(TK_ASTERISK)) {
            return new_untyped_node(ND_DEREF, unary(scope), NULL);

        } else if (consume(TK_AND)) {
            return new_untyped_node(ND_ADDR, unary(scope), NULL);
        }

        return primary(scope);
    }
}

// mul = unary ("*" unary | "/" unary)*
static UntypedNode mul(Scope scope) {
    UntypedNode untyped_node = unary(scope);

    for (;;) {
        if (consume(TK_ASTERISK)) {
            untyped_node =
                new_untyped_node(ND_MUL, untyped_node, unary(scope), NULL);

        } else if (consume(TK_SLASH)) {
            untyped_node =
                new_untyped_node(ND_DIV, untyped_node, unary(scope), NULL);

        } else {
            return untyped_node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
static UntypedNode add(Scope scope) {
    UntypedNode untyped_node = mul(scope);

    for (;;) {
        if (consume(TK_PLUS)) {
            untyped_node =
                new_untyped_node(ND_ADD, untyped_node, mul(scope), NULL);

        } else if (consume(TK_MINUS)) {
            untyped_node =
                new_untyped_node(ND_SUB, untyped_node, mul(scope), NULL);

        } else {
            return untyped_node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static UntypedNode relational(Scope scope) {
    UntypedNode untyped_node = add(scope);

    for (;;) {
        if (consume(TK_LESS)) {
            untyped_node =
                new_untyped_node(ND_LESS, untyped_node, add(scope), NULL);

        } else if (consume(TK_LESS_EQUAL)) {
            untyped_node = new_untyped_node(ND_LESS_OR_EQUAL, untyped_node,
                                            add(scope), NULL);

        } else if (consume(TK_MORE)) {
            untyped_node =
                new_untyped_node(ND_LESS, add(scope), untyped_node, NULL);

        } else if (consume(TK_MORE_EQUAL)) {
            untyped_node = new_untyped_node(ND_LESS_OR_EQUAL, add(scope),
                                            untyped_node, NULL);

        } else {
            return untyped_node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
static UntypedNode equality(Scope scope) {
    UntypedNode untyped_node = relational(scope);

    for (;;) {
        if (consume(TK_EQUAL_EQUAL)) {
            untyped_node = new_untyped_node(ND_EQUAL, untyped_node,
                                            relational(scope), NULL);

        } else if (consume(TK_EXCL_EQUAL)) {
            untyped_node = new_untyped_node(ND_NOT_EQUAL, untyped_node,
                                            relational(scope), NULL);

        } else {
            return untyped_node;
        }
    }
}
// assign = equality ("=" assign)?
static UntypedNode assign(Scope scope) {
    UntypedNode untyped_node = equality(scope);

    if (consume(TK_EQUAL)) {
        untyped_node =
            new_untyped_node(ND_ASSIGN, untyped_node, assign(scope), NULL);
    }

    return untyped_node;
}

// expr = assign
static UntypedNode expr(Scope scope) { return assign(scope); }

// stmt = "int" ( "*" )* ident ";"
//      | "int" ident ";"
//      | ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
//      | "int" ident ";"
//      | expr ";"
static UntypedNode stmt(Scope scope) {
    // "int" ident ";"
    if (consume(TK_INT)) {
        Type type = new_type_int();

        for (;;) {
            if (consume(TK_ASTERISK)) {
                type = new_type_ptr(type);

            } else {
                scope_def_local_var(scope, type, expect_ident());
                break;
            }
        }
        expect(TK_SEMICOLON);
        return new_untyped_node_null();
    }

    // ";"
    if (consume(TK_SEMICOLON)) {
        return new_untyped_node_null();
    }

    // "{" stmt* "}"
    if (consume(TK_LEFT_BRACE)) {
        Vec children = new_vec();

        Scope block_scope = new_scope(scope);
        while (!consume(TK_RIGHT_BRACE)) {
            vec_push(children, stmt(block_scope));
        }

        return new_untyped_node_block(children);
    }

    // "if" "(" expr ")" stmt ("else" stmt)?
    if (consume(TK_IF)) {
        expect(TK_LEFT_PAREN);
        UntypedNode cond = expr(scope);
        expect(TK_RIGHT_PAREN);
        UntypedNode then = stmt(scope);

        if (consume(TK_ELSE)) {
            return new_untyped_node(ND_IF_ELSE, cond, then, stmt(scope), NULL);

        } else {
            return new_untyped_node(ND_IF, cond, then, NULL);
        }
    }

    // "while" "(" expr ")" stmt
    if (consume(TK_WHILE)) {
        expect(TK_LEFT_PAREN);
        UntypedNode cond = expr(scope);
        expect(TK_RIGHT_PAREN);

        return new_untyped_node(ND_WHILE, cond, stmt(scope), NULL);
    }

    // "for" "(" expr? ";" expr? ";" expr? ")" stmt
    if (consume(TK_FOR)) {
        expect(TK_LEFT_PAREN);

        UntypedNode init;
        if (consume(TK_SEMICOLON)) {
            init = new_untyped_node_null();

        } else {
            init = expr(scope);
            expect(TK_SEMICOLON);
        }

        UntypedNode cond;
        if (consume(TK_SEMICOLON)) {
            cond = new_untyped_node_null();

        } else {
            cond = expr(scope);
            expect(TK_SEMICOLON);
        }

        UntypedNode update;
        if (consume(TK_RIGHT_PAREN)) {
            update = new_untyped_node_null();

        } else {
            update = expr(scope);
            expect(TK_RIGHT_PAREN);
        }

        return new_untyped_node(ND_FOR, init, cond, update, stmt(scope), NULL);
    }

    // "return" expr ";"
    if (consume(TK_RETURN)) {
        UntypedNode untyped_node =
            new_untyped_node(ND_RETURN, expr(scope), NULL);
        expect(TK_SEMICOLON);
        return untyped_node;
    }

    // expr ";"
    UntypedNode untyped_node = expr(scope);
    expect(TK_SEMICOLON);
    return untyped_node;
}

// func = "int" ident "(" ("int" ident ",")* ")" "{" stmt* "}"
UntypedNode func(Scope scope) {
    expect(TK_INT);

    Type type = new_type_int();
    Vec children = new_vec();
    UntypedNode untyped_node;

    for (;;) {
        if (consume(TK_ASTERISK)) {
            type = new_type_ptr(type);

        } else {
            untyped_node = new_untyped_node_func(scope, new_type_func(type),
                                                 expect_ident(), children);
            break;
        }
    }

    Scope func_scope = new_scope_func(scope, untyped_node->item);

    expect(TK_LEFT_PAREN);
    if (!consume(TK_RIGHT_PAREN)) {
        do {
            expect(TK_INT);

            UntypedNode untyped_node;
            Type type = new_type_int();

            for (;;) {
                if (consume(TK_ASTERISK)) {
                    type = new_type_ptr(type);

                } else {
                    untyped_node = new_untyped_node_local_var_with_def(
                        func_scope, type, expect_ident());
                    break;
                }
            }

            vec_push(children, untyped_node);

        } while (consume(TK_COMMA));
        expect(TK_RIGHT_PAREN);
    }

    if (consume(TK_LEFT_BRACE)) {
        Vec func_block_children = new_vec();
        while (!consume(TK_RIGHT_BRACE)) {
            vec_push(func_block_children, stmt(func_scope));
        }
        vec_push(children, new_untyped_node_block(func_block_children));

        return untyped_node;

    } else {
        expect(TK_SEMICOLON);
        return NULL;
    }
}

// program = func*;
UntypedNode program() {
    Vec children = new_vec();
    Scope scope = new_scope_global();

    while (!consume(TK_EOF)) {
        UntypedNode fn = func(scope);
        if (fn != NULL) vec_push(children, fn);
    }

    return new_untyped_node_program(children);
}
