#include "compiler.h"

static int consume(TokenKind tk) {
    Token token = tokens_peek();
    if (token->kind != tk) return 0;
    tokens_next();
    return 1;
}

static int consume_ident_is(char *ident) {
    Token token = tokens_peek();
    if (token->kind != TK_IDENT) return 0;
    if (strcmp(token->str, ident)) return 0;
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

static void expect_ident_is(char *ident) {
    Token token = tokens_peek();
    if (token->kind != TK_IDENT)
        error_at(token->line, token->row, "識別子ではありません");
    if (strcmp(token->str, ident))
        error_at(token->line, token->row, "期待される識別子ではありません");
    tokens_next();
}

static char *expect_ident() {
    Token token = tokens_peek();
    if (token->kind != TK_IDENT)
        error_at(token->line, token->row, "識別子ではありません");
    tokens_next();
    return token->str;
}

static int expect_int() {
    Token token = tokens_peek();
    if (token->kind != TK_INT)
        error_at(token->line, token->row, "数ではありません");
    tokens_next();
    return token->val;
}

static Node expr(NameSpace name_space);

// primary = num
//         | ident ("(" ")")?
//         | "(" expr ")"
static Node primary(NameSpace name_space) {
    if (consume(/* ( */ TK_LEFT_PAREN)) {
        Node node = expr(name_space);
        expect(/* ) */ TK_RIGHT_PAREN);
        return node;
    }

    char *name;
    if ((name = consume_ident()) != NULL) {
        if (consume(TK_LEFT_PAREN)) {
            Vec params = new_vec();
            if (!consume(TK_RIGHT_PAREN)) {
                do {
                    vec_push(params, expr(name_space));
                } while (consume(TK_COMMA));
                expect(TK_RIGHT_PAREN);
            }

            Vec children = new_vec();

            int i;
            for (i = params->len - 1; i >= 0; i--) {
                vec_push(children, params->buf[i]);
            }

            return new_node_call(name, children);
        }

        int offset = name_space_get_local_var_offset(name_space, name);
        return new_node_local_var(offset);
    }

    // そうでなければ数値のはず
    Node node = new_node_const(expect_int());
    return node;
}

// unary = ("+" | "-")? primary
static Node unary(NameSpace name_space) {
    for (;;) {
        if (consume(/* + */ TK_PLUS)) {
            return primary(name_space);

        } else if (consume(/* - */ TK_MINUS)) {
            return new_node(ND_SUB, new_node_const(0), primary(name_space),
                            NULL);
        }

        return primary(name_space);
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node mul(NameSpace name_space) {
    Node node = unary(name_space);

    for (;;) {
        if (consume(/* * */ TK_ASTERISK)) {
            node = new_node(ND_MUL, node, unary(name_space), NULL);

        } else if (consume(/* / */ TK_SLASH)) {
            node = new_node(ND_DIV, node, unary(name_space), NULL);

        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
static Node add(NameSpace name_space) {
    Node node = mul(name_space);

    for (;;) {
        if (consume(/* + */ TK_PLUS)) {
            node = new_node(ND_ADD, node, mul(name_space), NULL);

        } else if (consume(/* - */ TK_MINUS)) {
            node = new_node(ND_SUB, node, mul(name_space), NULL);

        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node relational(NameSpace name_space) {
    Node node = add(name_space);

    for (;;) {
        if (consume(/* < */ TK_LESS)) {
            node = new_node(ND_LESS, node, add(name_space), NULL);

        } else if (consume(/* <= */ TK_LESS_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, node, add(name_space), NULL);

        } else if (consume(/* > */ TK_MORE)) {
            node = new_node(ND_LESS, add(name_space), node, NULL);

        } else if (consume(/* >= */ TK_MORE_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, add(name_space), node, NULL);

        } else {
            return node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
static Node equality(NameSpace name_space) {
    Node node = relational(name_space);

    for (;;) {
        if (consume(/* == */ TK_EQUAL_EQUAL)) {
            node = new_node(ND_EQUAL, node, relational(name_space), NULL);

        } else if (consume(/* != */ TK_EXCL_EQUAL)) {
            node = new_node(ND_NOT_EQUAL, node, relational(name_space), NULL);

        } else {
            return node;
        }
    }
}
// assign = equality ("=" assign)?
static Node assign(NameSpace name_space) {
    Node node = equality(name_space);

    if (consume(/* = */ TK_EQUAL)) {
        node = new_node(ND_ASSIGN, node, assign(name_space), NULL);
    }

    return node;
}

// expr = assign
static Node expr(NameSpace name_space) { return assign(name_space); }

// stmt = "int" ident ";"
//      | ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
//      | "int" ident ";"
//      | expr ";"
static Node stmt(NameSpace name_space) {
    // "int" ident ";"
    if (consume_ident_is("int")) {
        name_space_def_local_var(name_space, expect_ident());
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

        NameSpace block_name_space = new_name_space(name_space);
        while (!consume(TK_RIGHT_BRACE)) {
            vec_push(children, stmt(block_name_space));
        }

        return new_node_block(children);
    }

    // "if" "(" expr ")" stmt ("else" stmt)?
    if (consume(TK_IF)) {
        expect(TK_LEFT_PAREN);
        Node cond = expr(name_space);
        expect(TK_RIGHT_PAREN);
        Node then = stmt(name_space);

        if (consume(TK_ELSE)) {
            return new_node(ND_IF_ELSE, cond, then, stmt(name_space), NULL);

        } else {
            return new_node(ND_IF, cond, then, NULL);
        }
    }

    // "while" "(" expr ")" stmt
    if (consume(TK_WHILE)) {
        expect(TK_LEFT_PAREN);
        Node cond = expr(name_space);
        expect(TK_RIGHT_PAREN);

        return new_node(ND_WHILE, cond, stmt(name_space), NULL);
    }

    // "for" "(" expr? ";" expr? ";" expr? ")" stmt
    if (consume(TK_FOR)) {
        expect(TK_LEFT_PAREN);

        Node init;
        if (consume(TK_SEMICOLON)) {
            init = new_node_null();
        } else {
            init = expr(name_space);
            expect(TK_SEMICOLON);
        }

        Node cond;
        if (consume(TK_SEMICOLON)) {
            cond = new_node_null();
        } else {
            cond = expr(name_space);
            expect(TK_SEMICOLON);
        }

        Node update;
        if (consume(TK_RIGHT_PAREN)) {
            update = new_node_null();
        } else {
            update = expr(name_space);
            expect(TK_RIGHT_PAREN);
        }

        return new_node(ND_FOR, init, cond, update, stmt(name_space), NULL);
    }

    // "return" expr ";"
    if (consume(TK_RETURN)) {
        Node node = new_node(ND_RETURN, expr(name_space), NULL);
        expect(TK_SEMICOLON);
        return node;
    }

    // expr ";"
    Node node = expr(name_space);
    expect(TK_SEMICOLON);
    return node;
}

#define MAX_ARGS 6

// func = "int" ident "(" ("int" ident ",")* ")" "{" stmt* "}"
Node func(NameSpace name_space) {
    int i;
    expect_ident_is("int");

    char *name = expect_ident();
    name_space_def_func(name_space, name);
    NameSpace func_name_space = new_name_space(name_space);

    Vec arg_names = new_vec();
    expect(TK_LEFT_PAREN);
    if (!consume(TK_RIGHT_PAREN)) {
        do {
            expect_ident_is("int");
            vec_push(arg_names, expect_ident());
        } while (consume(TK_COMMA));
        expect(TK_RIGHT_PAREN);
    }

    Vec args = new_vec();
    for (i = arg_names->len - 1; i >= 0; i--) {
        char *name = arg_names->buf[i];
        name_space_def_local_var(func_name_space, name);
        int offset = name_space_get_local_var_offset(func_name_space, name);
        vec_push(args, new_node_local_var(offset));
    }

    Vec child0_children = new_vec();
    expect(TK_LEFT_BRACE);
    while (!consume(TK_RIGHT_BRACE)) {
        vec_push(child0_children, stmt(func_name_space));
    }

    Vec children = new_vec();
    vec_push(children, new_node_block(child0_children));

    for (i = 0; i < args->len; i++) {
        vec_push(children, args->buf[i]);
    }

    return new_node_func(name, func_name_space->size, children);
}

// program = func*;
Node program() {
    Vec children = new_vec();
    NameSpace name_space = new_name_space(NULL);

    while (!consume(TK_EOF)) {
        vec_push(children, func(name_space));
    }

    return new_node_program(children);
}
