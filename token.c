#include "compiler.h"

static Token new_token_uncheckd(TokenKind tk, int line, int row) {
    Token token = checkd_malloc(sizeof(*token));
    token->kind = tk;
    token->line = line;
    token->row = row;
    return token;
}

Token new_token(TokenKind tk, int line, int row) {
    if (tk == TK_INT) error("new_token: Use new_token_int");
    if (tk == TK_IDENT) error("new_token: Use new_token_ident");
    Token token = new_token_uncheckd(tk, line, row);
    return token;
}

Token new_token_int(int val, int line, int row) {
    Token token = new_token_uncheckd(TK_INT, line, row);
    token->val = val;
    return token;
}

Token new_token_ident(char *str, int line, int row) {
    Token token = new_token_uncheckd(TK_INT, line, row);
    token->str = str;
    return token;
}
