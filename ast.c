#include "compiler.h"

static Node *ast;

static Node *primary();
static Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
static Node *new_node_num(int val);
static Node *mul();
static Node *expr();

static Node *primary() {
    // 次のトークンが"("なら、"(" expr ")"のはず
    if (ts_consume('(')) {
        Node *node = expr();
        ts_expect(')');
        return node;
    }

    // そうでなければ数値のはず
    return new_node_num(ts_expect_number());
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

static Node *mul() {
    Node *node = primary();

    for (;;) {
        if (ts_consume('*'))
            node = new_node(ND_MUL, node, primary());
        else if (ts_consume('/'))
            node = new_node(ND_DIV, node, primary());
        else
            return node;
    }
}

static Node *expr() {
    Node *node = mul();

    for (;;) {
        if (ts_consume('+'))
            node = new_node(ND_ADD, node, mul());
        else if (ts_consume('-'))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

void ast_init() { ast = expr(); };

void ast_compile() { gen(ast); }
