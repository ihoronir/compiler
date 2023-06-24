#include "compiler.h"

#define TOKENS_INITIAL_CAPACITY 100

static Vec tokens = NULL;
static int pos = 0;

static void tokens_push_token(Token token) {
    if (tokens == NULL) tokens = new_vec();
    vec_push(tokens, token);
}

void tokens_push(TokenKind kind, int line, int row) {
    tokens_push_token(new_token(kind, line, row));
}

void tokens_push_const_int(int val, int line, int row) {
    tokens_push_token(new_token_const_int(val, line, row));
}

void tokens_push_ident(char *str, int line, int row) {
    tokens_push_token(new_token_ident(str, line, row));
}

Token tokens_next() { return tokens->buf[pos++]; }
Token tokens_peek() { return tokens->buf[pos]; }
