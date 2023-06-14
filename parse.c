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

static int expect_int() {
    Token token = tokens_peek();
    if (token->kind != TK_INT)
        error_at(token->line, token->row, "数ではありません");
    tokens_next();
    return token->val;
}

static Node expr();

// primary = num | ident | "(" expr ")"
static Node primary() {
    if (consume(/* ( */ TK_LEFT_PAREN)) {
        Node node = expr();
        expect(/* ) */ TK_RIGHT_PAREN);
        return node;
    }

    char *str;
    if ((str = consume_ident()) != NULL) {
        int offset = get_offset(str);

        return new_node_local_var(offset);
    }

    // そうでなければ数値のはず
    Node node = new_node_const(expect_int());
    return node;
}

// unary = ("+" | "-")? primary
static Node unary() {
    for (;;) {
        if (consume(/* + */ TK_PLUS)) {
            return primary();

        } else if (consume(/* - */ TK_MINUS)) {
            return new_node(ND_SUB, new_node_const(0), primary(), NULL);
        }

        return primary();
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node mul() {
    Node node = unary();

    for (;;) {
        if (consume(/* * */ TK_ASTERISK)) {
            node = new_node(ND_MUL, node, unary(), NULL);

        } else if (consume(/* / */ TK_SLASH)) {
            node = new_node(ND_DIV, node, unary(), NULL);

        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
static Node add() {
    Node node = mul();

    for (;;) {
        if (consume(/* + */ TK_PLUS)) {
            node = new_node(ND_ADD, node, mul(), NULL);

        } else if (consume(/* - */ TK_MINUS)) {
            node = new_node(ND_SUB, node, mul(), NULL);

        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node relational() {
    Node node = add();

    for (;;) {
        if (consume(/* < */ TK_LESS)) {
            node = new_node(ND_LESS, node, add(), NULL);

        } else if (consume(/* <= */ TK_LESS_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, node, add(), NULL);

        } else if (consume(/* > */ TK_MORE)) {
            node = new_node(ND_LESS, add(), node, NULL);

        } else if (consume(/* >= */ TK_MORE_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, add(), node, NULL);

        } else {
            return node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
static Node equality() {
    Node node = relational();

    for (;;) {
        if (consume(/* == */ TK_EQUAL_EQUAL)) {
            node = new_node(ND_EQUAL, node, relational(), NULL);

        } else if (consume(/* != */ TK_EXCL_EQUAL)) {
            node = new_node(ND_NOT_EQUAL, node, relational(), NULL);

        } else {
            return node;
        }
    }
}
// assign = equality ("=" assign)?
static Node assign() {
    Node node = equality();

    if (consume(/* = */ TK_EQUAL)) {
        node = new_node(ND_ASSIGN, node, assign(), NULL);
    }

    return node;
}

// expr = assign
static Node expr() { return assign(); }

// stmt = ";"
//      | "{" stmt* "}"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";" expr? ")" stmt
//      | "return" expr ";"
//      | expr ";"
static Node stmt() {
    // ";"
    if (consume(TK_SEMICOLON)) {
        return new_node_null();
    }

    // "{" stmt* "}"
    if (consume(TK_LEFT_BRACE)) {
        Vec children = new_vec();

        while (!consume(TK_RIGHT_BRACE)) {
            vec_push(children, stmt());
        }

        return new_node_block(children);
    }

    // "if" "(" expr ")" stmt ("else" stmt)?
    if (consume(TK_IF)) {
        expect(TK_LEFT_PAREN);
        Node cond = expr();
        expect(TK_RIGHT_PAREN);
        Node then = stmt();

        if (consume(TK_ELSE)) {
            return new_node(ND_IF_ELSE, cond, then, stmt(), NULL);

        } else {
            return new_node(ND_IF, cond, then, NULL);
        }
    }

    // "while" "(" expr ")" stmt
    if (consume(TK_WHILE)) {
        expect(TK_LEFT_PAREN);
        Node cond = expr();
        expect(TK_RIGHT_PAREN);

        return new_node(ND_WHILE, cond, stmt(), NULL);
    }

    // "for" "(" expr? ";" expr? ";" expr? ")" stmt
    if (consume(TK_FOR)) {
        expect(TK_LEFT_PAREN);

        Node init;
        if (consume(TK_SEMICOLON)) {
            init = new_node_null();
        } else {
            init = expr();
            expect(TK_SEMICOLON);
        }

        Node cond;
        if (consume(TK_SEMICOLON)) {
            cond = new_node_null();
        } else {
            cond = expr();
            expect(TK_SEMICOLON);
        }

        Node update;
        if (consume(TK_RIGHT_PAREN)) {
            update = new_node_null();
        } else {
            update = expr();
            expect(TK_RIGHT_PAREN);
        }

        return new_node(ND_FOR, init, cond, update, stmt(), NULL);
    }

    // "return" expr ";"
    if (consume(TK_RETURN)) {
        Node node = new_node(ND_RETURN, expr(), NULL);
        expect(TK_SEMICOLON);
        return node;
    }

    // expr ";"
    Node node = expr();
    expect(TK_SEMICOLON);
    return node;
}

#define MAX_ARGS 6

// func = ident "(" (ident ",")* ")" "{" stmt* "}"
Node func() {
    char *name = expect_ident();

    expect(TK_LEFT_PAREN);
    Vec args = new_vec();
    if (!consume(TK_RIGHT_PAREN)) {
        do {
            vec_push(args, expect_ident());
        } while (consume(TK_COMMA));
        expect(TK_RIGHT_PAREN);
    }

    Vec child0_children = new_vec();
    expect(TK_LEFT_BRACE);
    while (!consume(TK_RIGHT_BRACE)) {
        vec_push(child0_children, stmt());
    }

    Vec children = new_vec();
    vec_push(children, new_node_block(child0_children));

    int i;
    for (i = args->len - 1; i >= 0; i--) {
        vec_push(children, new_node_local_var(get_offset(args->buf[i])));
    }

    return new_node_func(name, children);
}

// program = func*;
Node program() {
    Vec children = new_vec();

    while (!consume(TK_EOF)) {
        vec_push(children, func());
    }

    return new_node_program(children);
}
