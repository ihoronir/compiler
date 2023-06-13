#include "compiler.h"

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

// 入力文字列pをトークナイズ
void tokenize(char *p) {
    int line = 1;
    int row = 0;

    while (*p) {
        // 空白文字をスキップ
        switch (*p) {
            case '\n':
                line++;
                row = 0;
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
            tokens_push(TK_LESS_EQUAL, line, row);
            p += 2;
            continue;
        }

        if (!strncmp(p, ">=", 2)) {
            tokens_push(TK_MORE_EQUAL, line, row);
            p += 2;
            continue;
        }

        if (!strncmp(p, "==", 2)) {
            tokens_push(TK_EQUAL_EQUAL, line, row);
            p += 2;
            continue;
        }

        if (!strncmp(p, "!=", 2)) {
            tokens_push(TK_EXCL_EQUAL, line, row);
            p += 2;
            continue;
        }

        switch (*p) {
            case '+':
                tokens_push(TK_PLUS, line, row);
                p++;
                continue;
            case '-':
                tokens_push(TK_MINUS, line, row);
                p++;
                continue;
            case '*':
                tokens_push(TK_ASTERISK, line, row);
                p++;
                continue;
            case '/':
                tokens_push(TK_SLASH, line, row);
                p++;
                continue;
            case '(':
                tokens_push(TK_LEFT_PAREN, line, row);
                p++;
                continue;
            case ')':
                tokens_push(TK_RIGHT_PAREN, line, row);
                p++;
                continue;
            case '<':
                tokens_push(TK_LESS, line, row);
                p++;
                continue;
            case '>':
                tokens_push(TK_MORE, line, row);
                p++;
                continue;
            case '=':
                tokens_push(TK_EQUAL, line, row);
                p++;
                continue;
            case ';':
                tokens_push(TK_SEMICOLON, line, row);
                p++;
                continue;
        }

        if (isdigit(*p)) {
            tokens_push_int(strtol(p, &p, 10), line, row);
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            tokens_push(TK_RETURN, line, row);
            p += 6;
            continue;
        }

        char *str;
        if ((str = identifier(&p)) != NULL) {
            tokens_push_ident(str, line, row);
            continue;
        }

        error_at(line, row, "トークナイズできません");
    }

    tokens_push(TK_EOF, line, row);
}
