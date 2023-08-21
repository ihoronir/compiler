#include "compiler.h"

static int is_alnum(char c) {
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') ||
           ('0' <= c && c <= '9') || (c == '_');
}

static char *identifier(char **p) {
    String string = new_string();

    while (is_alnum(**p)) {
        string_push(string, *((*p)++));
    }

    if (string->len == 0) return NULL;

    string_push(string, '\0');

    return string->buf;
}

void tokenize(char *p) {
    char *row_head = p;
    struct position position;
    position.row = 1;

    while (*p != '\0') {
        switch (*p) {
            case '\n':
                position.row++;
                row_head = ++p;
                continue;

            case '\t':
            case '\r':
            case ' ':
                p++;
                continue;

            default:
                position.column = p - row_head + 1;
                break;
        }

        // 行コメントをスキップ
        if (strncmp(p, "//", 2) == 0) {
            p += 2;
            while (*p != '\n') p++;
            continue;
        }

        // ブロックコメントをスキップ
        if (strncmp(p, "/*", 2) == 0) {
            struct position start_pos = position;
            p += 2;

            for (;;) {
                if (*p == '\0') {
                    error_at(start_pos, "コメントがとじられていません");
                }

                if (strncmp(p, "*/", 2) == 0) {
                    p += 2;
                    break;
                }

                if (*p == '\n') {
                    position.row++;
                    row_head = ++p;

                } else {
                    p++;
                }
            }
            continue;
        }

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            tokens_push(TK_SIZEOF, position);
            p += 6;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            tokens_push(TK_RETURN, position);
            p += 6;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            tokens_push(TK_WHILE, position);
            p += 5;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            tokens_push(TK_ELSE, position);
            p += 4;
            continue;
        }

        if (strncmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
            tokens_push(TK_CHAR, position);
            p += 4;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            tokens_push(TK_FOR, position);
            p += 3;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            tokens_push(TK_INT, position);
            p += 3;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            tokens_push(TK_IF, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, "<=", 2)) {
            tokens_push(TK_LESS_EQUAL, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, ">=", 2)) {
            tokens_push(TK_MORE_EQUAL, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, "<<", 2)) {
            tokens_push(TK_LESS_LESS, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, ">>", 2)) {
            tokens_push(TK_MORE_MORE, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, "==", 2)) {
            tokens_push(TK_EQUAL_EQUAL, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, "!=", 2)) {
            tokens_push(TK_EXCL_EQUAL, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, "+=", 2)) {
            tokens_push(TK_PLUS_EQUAL, position);
            p += 2;
            continue;
        }

        if (!strncmp(p, "++", 2)) {
            tokens_push(TK_PLUS_PLUS, position);
            p += 2;
            continue;
        }

        switch (*p) {
            case '+':
                tokens_push(TK_PLUS, position);
                p++;
                continue;
            case '-':
                tokens_push(TK_MINUS, position);
                p++;
                continue;
            case '*':
                tokens_push(TK_ASTERISK, position);
                p++;
                continue;
            case '/':
                tokens_push(TK_SLASH, position);
                p++;
                continue;
            case '%':
                tokens_push(TK_PERCENT, position);
                p++;
                continue;
            case '(':
                tokens_push(TK_LEFT_PAREN, position);
                p++;
                continue;
            case ')':
                tokens_push(TK_RIGHT_PAREN, position);
                p++;
                continue;
            case '{':
                tokens_push(TK_LEFT_BRACE, position);
                p++;
                continue;
            case '}':
                tokens_push(TK_RIGHT_BRACE, position);
                p++;
                continue;
            case '[':
                tokens_push(TK_LEFT_BRACKET, position);
                p++;
                continue;
            case ']':
                tokens_push(TK_RIGHT_BRACKET, position);
                p++;
                continue;
            case '<':
                tokens_push(TK_LESS, position);
                p++;
                continue;
            case '>':
                tokens_push(TK_MORE, position);
                p++;
                continue;
            case '=':
                tokens_push(TK_EQUAL, position);
                p++;
                continue;
            case ';':
                tokens_push(TK_SEMICOLON, position);
                p++;
                continue;
            case ',':
                tokens_push(TK_COMMA, position);
                p++;
                continue;
            case '&':
                tokens_push(TK_AND, position);
                p++;
                continue;
        }

        if (*p == '"') {
            p++;

            String str = new_string();

            for (;;) {
                if (*p == '\0' || *p == '\n') {
                    error_at(position, "文字列リテラルが閉じられていません");
                }

                if (*p == '"') {
                    p++;
                    tokens_push_string(str->buf, position);
                    break;
                }

                string_push(str, *(p++));
            }

            continue;
        }

        if (*p == '\'') {
            p++;
            tokens_push_const_int(*(p++), position);

            if (*(p++) != '\'') {
                error_at(position, "文字定数が閉じられていません");
            }
            continue;
        }

        if (isdigit(*p)) {
            tokens_push_const_int(strtol(p, &p, 10), position);
            continue;
        }

        char *str;
        if ((str = identifier(&p)) != NULL) {
            tokens_push_ident(str, position);
            continue;
        }

        error_at(position, "トークナイズできません");
    }

    tokens_push(TK_EOF, position);
}
