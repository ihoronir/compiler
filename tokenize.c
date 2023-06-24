#include "compiler.h"

static int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

static char *identifier(char **p) {
    String string = new_string();

    while (isalnum(**p)) {
        string_push(string, *((*p)++));
    }

    if (string->len == 0) return NULL;

    string_push(string, '\0');

    return string->buf;
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

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            tokens_push(TK_RETURN, line, row);
            p += 6;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            tokens_push(TK_WHILE, line, row);
            p += 5;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            tokens_push(TK_ELSE, line, row);
            p += 4;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            tokens_push(TK_FOR, line, row);
            p += 3;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            tokens_push(TK_INT, line, row);
            p += 3;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            tokens_push(TK_IF, line, row);
            p += 2;
            continue;
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
            case '{':
                tokens_push(TK_LEFT_BRACE, line, row);
                p++;
                continue;
            case '}':
                tokens_push(TK_RIGHT_BRACE, line, row);
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
            case ',':
                tokens_push(TK_COMMA, line, row);
                p++;
                continue;
        }

        if (isdigit(*p)) {
            tokens_push_const_int(strtol(p, &p, 10), line, row);
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
