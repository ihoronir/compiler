#include "compiler.h"

#define TOKENS_BUF_INITIAL_CAPACITY 100

static Token *tokens_buf;
static int tokens_buf_len;
static int tokens_buf_capacity;
static int tokens_buf_pos;

static void tokens_buf_init() {
    tokens_buf = checkd_malloc(sizeof(Token) * TOKENS_BUF_INITIAL_CAPACITY);
    tokens_buf_len = 0;
    tokens_buf_capacity = TOKENS_BUF_INITIAL_CAPACITY;
    tokens_buf_pos = 0;
}

static Token *tokens_buf_ref_to_last() {
    return &tokens_buf[tokens_buf_len - 1];
}

static void tokens_buf_push_uncheckd(TokenKind tk, int line, int row) {
    Token token;
    token.kind = tk;
    token.line = line;
    token.row = row;

    if (tokens_buf_len == tokens_buf_capacity) {
        tokens_buf_capacity *= 2;
        tokens_buf =
            checkd_realloc(tokens_buf, sizeof(Token) * tokens_buf_capacity * 2);
    }
    tokens_buf[tokens_buf_len++] = token;
}

static void tokens_buf_push(TokenKind tk, int line, int row) {
    if (tk == TK_NUM) error("new_token: Use new_token_num");
    if (tk == TK_IDENTIFIER) error("new_token: Use new_token_identifier");
    return tokens_buf_push_uncheckd(tk, line, row);
}

static void tokens_buf_push_num(int val, int line, int row) {
    tokens_buf_push_uncheckd(TK_NUM, line, row);
    tokens_buf_ref_to_last()->val = val;
}

static void tokens_buf_push_identifier(char *str, int line, int row) {
    tokens_buf_push_uncheckd(TK_IDENTIFIER, line, row);
    tokens_buf_ref_to_last()->str = str;
}

static int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

#define IDENTIFIER_MAX_LEN 63

static char *identifier(char **p) {
    static char buf[IDENTIFIER_MAX_LEN];

    int i;
    for (i = 0; i < IDENTIFIER_MAX_LEN; i++) {
        if (!isalnum(**p)) break;
        buf[i] = *((*p)++);
    }

    if (i == 0) return NULL;

    char *str = checkd_malloc(sizeof(char) * (i + 1));
    strncpy(str, buf, i + 1);
    return str;
}

Token *token_next() { return &tokens_buf[tokens_buf_pos++]; }
Token *token_peek() { return &tokens_buf[tokens_buf_pos]; }

// 入力文字列pをトークナイズ
void tokenize(char *p) {
    int line = 1;
    int row = 0;

    tokens_buf_init();

    while (*p) {
        // 空白文字をスキップ
        switch (*p) {
            case '\n':
                line++;
                p++;
                continue;

            case '\t':
                row += 4;  // タブ幅は 4 と仮定
                p++;
                continue;

            case '\r':
            case ' ':
                row++;
                p++;
                continue;

            default:
                row++;
                break;
        }

        if (!strncmp(p, "<=", 2)) {
            tokens_buf_push(TK_LESS_OR_EQUAL, line, row);
            p += 2;
            continue;
        }

        if (!strncmp(p, ">=", 2)) {
            tokens_buf_push(TK_MORE_OR_EQUAL, line, row);
            p += 2;
            continue;
        }

        if (!strncmp(p, "==", 2)) {
            tokens_buf_push(TK_EQUAL, line, row);
            p += 2;
            continue;
        }

        if (!strncmp(p, "!=", 2)) {
            tokens_buf_push(TK_NOT_EQUAL, line, row);
            p += 2;
            continue;
        }

        switch (*p) {
            case '+':
                tokens_buf_push(TK_PLUS, line, row);
                p++;
                continue;
            case '-':
                tokens_buf_push(TK_MINUS, line, row);
                p++;
                continue;
            case '*':
                tokens_buf_push(TK_ASTERISK, line, row);
                p++;
                continue;
            case '/':
                tokens_buf_push(TK_SLASH, line, row);
                p++;
                continue;
            case '(':
                tokens_buf_push(TK_LEFT_PARENTHESES, line, row);
                p++;
                continue;
            case ')':
                tokens_buf_push(TK_RIGHT_PARENTHESES, line, row);
                p++;
                continue;
            case '<':
                tokens_buf_push(TK_LESS, line, row);
                p++;
                continue;
            case '>':
                tokens_buf_push(TK_MORE, line, row);
                p++;
                continue;
        }

        if (isdigit(*p)) {
            tokens_buf_push_num(strtol(p, &p, 10), line, row);
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            tokens_buf_push(TK_RETURN, line, row);
            p += 6;
            continue;
        }

        char *str;
        if ((str = identifier(&p)) != NULL) {
            tokens_buf_push_identifier(str, line, row);
            continue;
        }

        error_at(line, row, "トークナイズできません");
    }

    tokens_buf_push(TK_EOF, line, row);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
// void ts_expect(char *op) {
//    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
//        memcmp(token->str, op, token->len))
//        error_at(token->str, "'%c'ではありません", op);
//    token = token->next;
//}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
// int ts_expect_number() {
//    if (token->kind != TK_NUM) error_at(token->str, "数ではありません");
//    int val = token->val;
//    token = token->next;
//    return val;
//}
