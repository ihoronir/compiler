#include "compiler.h"

static Node *code[100];

static Node *ast;

static int consume(TokenKind tk) {
    Token *token = token_peek();
    if (token->kind != tk) return 0;
    token_next();
    return 1;
}

static void expect(TokenKind tk) {
    Token *token = token_peek();
    if (token->kind != tk)
        error_at(token->line, token->row, "期待される字句ではありません");
    token_next();
}

static int expect_number() {
    Token *token = token_peek();
    if (token->kind != TK_NUM)
        error_at(token->line, token->row, "数ではありません");
    token_next();
    return token->val;
}

static char *expect_identifier() {
    Token *token = token_peek();
    if (token->kind != TK_IDENTIFIER)
        error_at(token->line, token->row, "識別子ではありません");
    token_next();
    return token->str;
}

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs = lhs;
    node->rhs = rhs;
    return node;
}

static Node *new_node_num(int val) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val = val;
    return node;
}

static Node *expr();

// primary = num | ident | "(" expr ")"
static Node *primary() {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (consume(TK_LEFT_PARENTHESES)) {
        Node *node = expr();
        expect(TK_RIGHT_PARENTHESES);
        return node;
    }

    // そうでなければ数値のはず
    return new_node_num(expect_number());
}

// unary = ("+" | "-")? primary
static Node *unary() {
    for (;;) {
        if (consume(TK_PLUS))
            return primary();
        else if (consume(TK_MINUS))
            return new_node(ND_SUB, new_node_num(0), primary());
        return primary();
    }
}

// mul = unary ("*" unary | "/" unary)*
static Node *mul() {
    Node *node = unary();

    for (;;) {
        if (consume(TK_ASTERISK))
            node = new_node(ND_MUL, node, unary());
        else if (consume(TK_SLASH))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// add = mul ("+" mul | "-" mul)*
static Node *add() {
    Node *node = mul();

    for (;;) {
        if (consume(TK_PLUS))
            node = new_node(ND_ADD, node, mul());
        else if (consume(TK_MINUS))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
static Node *relational() {
    Node *node = add();

    for (;;) {
        if (consume(TK_LESS))
            node = new_node(ND_LT, node, add());
        else if (consume(TK_LESS_OR_EQUAL))
            node = new_node(ND_LE, node, add());
        else if (consume(TK_MORE))
            node = new_node(ND_LT, add(), node);
        else if (consume(TK_MORE_OR_EQUAL))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

// equality = relational ("==" relational | "!=" relational)*
static Node *equality() {
    Node *node = relational();

    for (;;) {
        if (consume(TK_EQUAL))
            node = new_node(ND_EQ, node, relational());
        else if (consume(TK_NOT_EQUAL))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

// program    = stmt*
// stmt       = expr ";"
// expr       = assign
// assign     = equality ("=" assign)?
static Node *expr() { return equality(); }

void ast_init() { ast = expr(); };

void ast_compile() { gen(ast); }
