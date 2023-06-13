#include "compiler.h"

Node *code[100];

static int consume(TokenKind tk) {
    Token *token = tokens_peek();
    if (token->kind != tk) return 0;
    tokens_next();
    return 1;
}

static char *consume_ident() {
    Token *token = tokens_peek();
    if (token->kind != TK_IDENT) return NULL;
    tokens_next();
    return token->str;
}

static void expect(TokenKind tk) {
    Token *token = tokens_peek();
    if (token->kind != tk)
        error_at(token->line, token->row, "期待される字句ではありません");
    tokens_next();
}

static int expect_int() {
    Token *token = tokens_peek();
    if (token->kind != TK_INT)
        error_at(token->line, token->row, "数ではありません");
    tokens_next();
    return token->val;
}

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = checkd_malloc(sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_node_local_var(int offset) {
    Node *node = checkd_malloc(sizeof(Node));
    node->kind = ND_LOCAL_VAR;
    node->offset = offset;
    return node;
}

static Node *new_node_const(int val) {
    Node *node = checkd_malloc(sizeof(Node));
    node->kind = ND_CONST;
    node->val = val;
    return node;
}

static Node *new_node_return(Node *lhs) {
    Node *node = checkd_malloc(sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = lhs;
    return node;
}

static Node *expr();

// primary = num | ident | "(" expr ")"
static Node *primary() {
    if (consume(/* ( */ TK_LEFT_PAREN)) {
        Node *node = expr();
        expect(/* ) */ TK_RIGHT_PAREN);
        return node;
    }

    char *str;
    if ((str = consume_ident()) != NULL) {
        int offset = get_offset(str);

        return new_node_local_var(offset);
    }

    // そうでなければ数値のはず
    return new_node_const(expect_int());
}

// unary = ("+" | "-")? primary
static Node *unary() {
    for (;;) {
        if (consume(/* + */ TK_PLUS)) {
            return primary();

        } else if (consume(/* - */ TK_MINUS)) {
            return new_node(ND_SUB, new_node_const(0), primary());
        }

        return primary();
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume(/* * */ TK_ASTERISK)) {
            node = new_node(ND_MUL, node, unary());

        } else if (consume(/* / */ TK_SLASH)) {
            node = new_node(ND_DIV, node, unary());

        } else {
            return node;
        }
    }
}

// add = mul ("+" mul | "-" mul)*
static Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume(/* + */ TK_PLUS)) {
            node = new_node(ND_ADD, node, mul());

        } else if (consume(/* - */ TK_MINUS)) {
            node = new_node(ND_SUB, node, mul());

        } else {
            return node;
        }
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume(/* < */ TK_LESS)) {
            node = new_node(ND_LESS, node, add());

        } else if (consume(/* <= */ TK_LESS_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, node, add());

        } else if (consume(/* > */ TK_MORE)) {
            node = new_node(ND_LESS, add(), node);

        } else if (consume(/* >= */ TK_MORE_EQUAL)) {
            node = new_node(ND_LESS_OR_EQUAL, add(), node);

        } else {
            return node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume(/* == */ TK_EQUAL_EQUAL)) {
            node = new_node(ND_EQUAL, node, relational());

        } else if (consume(/* != */ TK_EXCL_EQUAL)) {
            node = new_node(ND_NOT_EQUAL, node, relational());

        } else {
            return node;
        }
    }
}
// assign = equality ("=" assign)?
static Node *assign() {
    Node *node = equality();

    if (consume(/* = */ TK_EQUAL)) {
        node = new_node(ND_ASSIGN, node, assign());
    }

    return node;
}

// expr = assign
static Node *expr() { return assign(); }

// stmt = expr ";" | "return" expr ";"
static Node *stmt() {
    Node *node;

    if (consume(TK_RETURN)) {
        node = new_node_return(expr());

    } else {
        node = expr();
    }

    expect(TK_SEMICOLON);
    return node;
}

// program = stmt*
void program() {
    int i = 0;
    init_local_vars_buf();

    while (tokens_peek()->kind != TK_EOF) {
        code[i++] = stmt();
    }

    code[i] = NULL;
}
