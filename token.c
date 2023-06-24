#include "compiler.h"

static Token new_token_uncheckd(TokenKind kind, int line, int row) {
    Token token = checked_malloc(sizeof(*token));
    token->kind = kind;
    token->line = line;
    token->row = row;
    return token;
}

Token new_token(TokenKind kind, int line, int row) {
    if (kind == TK_CONST_INT) error("new_token: Use new_token_int");
    if (kind == TK_IDENT) error("new_token: Use new_token_ident");
    Token token = new_token_uncheckd(kind, line, row);
    return token;
}

Token new_token_const_int(int val_int, int line, int row) {
    Token token = new_token_uncheckd(TK_CONST_INT, line, row);
    token->val_int = val_int;
    return token;
}

Token new_token_ident(char *str, int line, int row) {
    Token token = new_token_uncheckd(TK_IDENT, line, row);
    token->str = str;
    return token;
}
