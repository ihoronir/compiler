#include "compiler.h"

#define TOKENS_INITIAL_CAPACITY 100

static Token *tokens;
static int len = 0;
static int capacity = 0;
static int pos = 0;

static void tokens_init() {
    tokens = checkd_malloc(sizeof(Token) * TOKENS_INITIAL_CAPACITY);
    capacity = TOKENS_INITIAL_CAPACITY;
}

static Token *tokens_ref_to_last() { return &tokens[len - 1]; }

static void tokens_push_uncheckd(TokenKind tk, int line, int row) {
    if (capacity == 0) tokens_init();

    if (len == capacity) {
        capacity *= 2;
        tokens = checkd_realloc(tokens, sizeof(Token) * capacity * 2);
    }

    Token token;
    token.kind = tk;
    token.line = line;
    token.row = row;
    tokens[len++] = token;
}

void tokens_push(TokenKind tk, int line, int row) {
    if (tk == TK_INT) error("tokens_push: Use tokens_push_int");
    if (tk == TK_IDENT) error("tokens_push: Use tokens_push_ident");
    return tokens_push_uncheckd(tk, line, row);
}

void tokens_push_int(int val, int line, int row) {
    tokens_push_uncheckd(TK_INT, line, row);
    tokens_ref_to_last()->val = val;
}

void tokens_push_ident(char *str, int line, int row) {
    tokens_push_uncheckd(TK_IDENT, line, row);
    tokens_ref_to_last()->str = str;
}

Token *tokens_next() { return &tokens[pos++]; }
Token *tokens_peek() { return &tokens[pos]; }
