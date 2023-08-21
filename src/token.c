#include "compiler.h"

static Token new_token_unchecked(TokenKind kind, struct position position) {
    Token token = checked_malloc(sizeof(*token));
    token->kind = kind;
    token->position = position;
    return token;
}

Token new_token(TokenKind kind, struct position position) {
    assert(kind != TK_CONST_INT);
    assert(kind != TK_IDENT);
    assert(kind != TK_STRING);
    Token token = new_token_unchecked(kind, position);
    return token;
}

Token new_token_const_int(int val_int, struct position position) {
    Token token = new_token_unchecked(TK_CONST_INT, position);
    token->val_int = val_int;
    return token;
}

Token new_token_ident(char *str, struct position position) {
    Token token = new_token_unchecked(TK_IDENT, position);
    token->str = str;
    return token;
}

Token new_token_string(char *str, struct position position) {
    Token token = new_token_unchecked(TK_STRING, position);
    token->str = str;
    return token;
}
