#include <stdio.h>

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

static Node expr(Scope scope);

// primary = num
//         | ident ("(" ")")?
//         | "(" expr ")"
static Node primary(Scope scope) {
    if (consume(TK_LEFT_PAREN)) {
        Node node = expr(scope);
        expect(TK_RIGHT_PAREN);
        return node;
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

            return new_node_call(scope, name, children);
        }

        return new_node_local_var(scope, name);
    }

    // そうでなければ数値のはず
    Node node = new_node_const_int(expect_const_int());
    return node;
}

// unary = ("+" | "-")? primary
static Node unary(Scope scope) {
    for (;;) {
        if (consume(TK_PLUS)) {
            return primary(scope);

        } else if (consume(TK_MINUS)) {
            return new_node(ND_SUB, new_node_const_int(0), primary(scope),
                            NULL);
        }

        return primary(scope);
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node mul(Scope scope) {
    Node node = unary(scope);

    for (;;) {
        if (consume(TK_ASTERISK)) {
            node = new_node(ND_MUL, node, unary(scope), NULL);

        } else if (consume(TK_SLASH)) {
            node = new_node(ND_DIV, node, unary(scope), NULL);

        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
static Node add(Scope scope) {
    Node node = mul(scope);

    for (;;) {
        if (consume(TK_PLUS)) {
            node = new_node(ND_ADD, node, mul(scope), NULL);

        } else if (consume(TK_MINUS)) {
            node = new_node(ND_SUB, node, mul(scope), NULL);

        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node relational(Scope scope) {
    Node node = add(scope);

    for (;;) {
        if (consume(TK_LESS)) {
            node = new_node(ND_LESS, node, add(scope), NULL);

        } else if (consume(TK_LESS_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, node, add(scope), NULL);

        } else if (consume(TK_MORE)) {
            node = new_node(ND_LESS, add(scope), node, NULL);

        } else if (consume(TK_MORE_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, add(scope), node, NULL);

        } else {
            return node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
static Node equality(Scope scope) {
    Node node = relational(scope);

    for (;;) {
        if (consume(TK_EQUAL_EQUAL)) {
            node = new_node(ND_EQUAL, node, relational(scope), NULL);

        } else if (consume(TK_EXCL_EQUAL)) {
            node = new_node(ND_NOT_EQUAL, node, relational(scope), NULL);

        } else {
            return node;
        }
    }
}
// assign = equality ("=" assign)?
static Node assign(Scope scope) {
    Node node = equality(scope);

    if (consume(/* = */ TK_EQUAL)) {
        node = new_node(ND_ASSIGN, node, assign(scope), NULL);
    }

    return node;
}

// expr = assign
static Node expr(Scope scope) { return assign(scope); }

// stmt = "int" ident ";"
//      | ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
//      | "int" ident ";"
//      | expr ";"
static Node stmt(Scope scope) {
    // "int" ident ";"
    if (consume(TK_INT)) {
        scope_def_local_var(scope, new_type_int(), expect_ident());
        expect(TK_SEMICOLON);
        return new_node_null();
    }

    // ";"
    if (consume(TK_SEMICOLON)) {
        return new_node_null();
    }

    // "{" stmt* "}"
    if (consume(TK_LEFT_BRACE)) {
        Vec children = new_vec();

        Scope block_scope = new_scope(scope);
        while (!consume(TK_RIGHT_BRACE)) {
            vec_push(children, stmt(block_scope));
        }

        return new_node_block(children);
    }

    // "if" "(" expr ")" stmt ("else" stmt)?
    if (consume(TK_IF)) {
        expect(TK_LEFT_PAREN);
        Node cond = expr(scope);
        expect(TK_RIGHT_PAREN);
        Node then = stmt(scope);

        if (consume(TK_ELSE)) {
            return new_node(ND_IF_ELSE, cond, then, stmt(scope), NULL);

        } else {
            return new_node(ND_IF, cond, then, NULL);
        }
    }

    // "while" "(" expr ")" stmt
    if (consume(TK_WHILE)) {
        expect(TK_LEFT_PAREN);
        Node cond = expr(scope);
        expect(TK_RIGHT_PAREN);

        return new_node(ND_WHILE, cond, stmt(scope), NULL);
    }

    // "for" "(" expr? ";" expr? ";" expr? ")" stmt
    if (consume(TK_FOR)) {
        expect(TK_LEFT_PAREN);

        Node init;
        if (consume(TK_SEMICOLON)) {
            init = new_node_null();
        } else {
            init = expr(scope);
            expect(TK_SEMICOLON);
        }

        Node cond;
        if (consume(TK_SEMICOLON)) {
            cond = new_node_null();
        } else {
            cond = expr(scope);
            expect(TK_SEMICOLON);
        }

        Node update;
        if (consume(TK_RIGHT_PAREN)) {
            update = new_node_null();
        } else {
            update = expr(scope);
            expect(TK_RIGHT_PAREN);
        }

        return new_node(ND_FOR, init, cond, update, stmt(scope), NULL);
    }

    // "return" expr ";"
    if (consume(TK_RETURN)) {
        Node node = new_node(ND_RETURN, expr(scope), NULL);
        expect(TK_SEMICOLON);
        return node;
    }

    // expr ";"
    Node node = expr(scope);
    expect(TK_SEMICOLON);
    return node;
}

// func = "int" ident "(" ("int" ident ",")* ")" "{" stmt* "}"
Node func(Scope scope) {
    expect(TK_INT);

    char *name = expect_ident();
    Vec children = new_vec();
    Node node = new_node_func(scope, new_type_func(), name, children);

    Scope func_scope = new_scope_func(scope, node->item);

    expect(TK_LEFT_PAREN);
    if (!consume(TK_RIGHT_PAREN)) {
        do {
            expect(TK_INT);
            vec_push(children, new_node_local_var_with_def(
                                   func_scope, new_type_int(), expect_ident()));
        } while (consume(TK_COMMA));
        expect(TK_RIGHT_PAREN);
    }

    Vec func_block_children = new_vec();
    expect(TK_LEFT_BRACE);
    while (!consume(TK_RIGHT_BRACE)) {
        vec_push(func_block_children, stmt(func_scope));
    }

    vec_push(children, new_node_block(func_block_children));

    return node;
}

// program = func*;
Node program() {
    Vec children = new_vec();
    Scope scope = new_scope_global();

    while (!consume(TK_EOF)) {
        vec_push(children, func(scope));
    }

    return new_node_program(children);
}
