#include "compiler.h"

static Node *ast;

static Node *expr();

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

static Node *primary() {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (ts_consume("(")) {
        Node *node = expr();
        ts_expect(")");
        return node;
    }

    // そうでなければ数値のはず
    return new_node_num(ts_expect_number());
}

static Node *unary() {
    for (;;) {
        if (ts_consume("+"))
            return primary();
        else if (ts_consume("-"))
            return new_node(ND_SUB, new_node_num(0), primary());
        return primary();
    }
}

static Node *mul() {
    Node *node = unary();

    for (;;) {
        if (ts_consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (ts_consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

static Node *add() {
    Node *node = mul();

    for (;;) {
        if (ts_consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (ts_consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}
static Node *relational() {
    Node *node = add();

    for (;;) {
        if (ts_consume("<"))
            node = new_node(ND_LT, node, add());
        else if (ts_consume("<="))
            node = new_node(ND_LE, node, add());
        else if (ts_consume(">"))
            node = new_node(ND_LT, add(), node);
        else if (ts_consume(">="))
            node = new_node(ND_LE, add(), node);
        else
            return node;
    }
}

static Node *equality() {
    Node *node = relational();

    for (;;) {
        if (ts_consume("=="))
            node = new_node(ND_EQ, node, relational());
        else if (ts_consume("!="))
            node = new_node(ND_NE, node, relational());
        else
            return node;
    }
}

static Node *expr() { return equality(); }

void ast_init() { ast = expr(); };

void ast_compile() { gen(ast); }
