#include "compiler.h"

static Token new_token_unchecked(TokenKind kind, int line, int column) {
    Token token = checked_malloc(sizeof(*token));
    token->kind = kind;
    token->line = line;
    token->column = column;
    return token;
}

Token new_token(TokenKind kind, int line, int column) {
    if (kind == TK_CONST_INT) error("new_token: Use new_token_int");
    if (kind == TK_IDENT) error("new_token: Use new_token_ident");
    Token token = new_token_unchecked(kind, line, column);
    return token;
}

Token new_token_const_int(int val_int, int line, int column) {
    Token token = new_token_unchecked(TK_CONST_INT, line, column);
    token->val_int = val_int;
    return token;
}

Token new_token_ident(char *str, int line, int column) {
    Token token = new_token_unchecked(TK_IDENT, line, column);
    token->str = str;
    return token;
}

Token new_token_string(char *str, int line, int column) {
    Token token = new_token_unchecked(TK_STRING, line, column);
    token->str = str;
    return token;
}
