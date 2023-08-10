#include "compiler.h"

#define TOKENS_INITIAL_CAPACITY 300

static Vec tokens = NULL;
static int pos = 0;

static void tokens_push_token(Token token) {
    if (tokens == NULL) tokens = new_vec_with_capacity(TOKENS_INITIAL_CAPACITY);
    vec_push(tokens, token);
}

void tokens_push(TokenKind kind, int line, int column) {
    tokens_push_token(new_token(kind, line, column));
}

void tokens_push_const_int(int val, int line, int column) {
    tokens_push_token(new_token_const_int(val, line, column));
}

void tokens_push_ident(char *str, int line, int column) {
    tokens_push_token(new_token_ident(str, line, column));
}

void tokens_push_string(char *str, int line, int column) {
    tokens_push_token(new_token_string(str, line, column));
}

Token tokens_next() { return tokens->buf[pos++]; }
Token tokens_peek() { return tokens->buf[pos]; }
