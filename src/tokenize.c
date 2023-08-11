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

// 入力文字列pをトークナイズ
void tokenize(char *p) {
    char *head_of_line = p;
    int line = 1;
    int column;

    while (*p) {
        // 空白文字をスキップ
        switch (*p) {
            case '\n':
                line++;
                head_of_line = ++p;
                continue;

            case '\t':
            case '\r':
            case ' ':
                p++;
                continue;

            default:
                column = p - head_of_line + 1;
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
            p += 2;
            for (;;) {
                if (*p == '\0') error("コメントがとじられていません");

                if (strncmp(p, "*/", 2) == 0) {
                    p += 2;
                    break;
                }

                if (*p == '\n') {
                    line++;
                    head_of_line = ++p;

                } else {
                    p++;
                }
            }

            // if (!q) error_at(p, "コメントが閉じられていません");
            // p = q + 2;
            continue;
        }

        if (strncmp(p, "sizeof", 6) == 0 && !is_alnum(p[6])) {
            tokens_push(TK_SIZEOF, line, column);
            p += 6;
            continue;
        }

        if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
            tokens_push(TK_RETURN, line, column);
            p += 6;
            continue;
        }

        if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
            tokens_push(TK_WHILE, line, column);
            p += 5;
            continue;
        }

        if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
            tokens_push(TK_ELSE, line, column);
            p += 4;
            continue;
        }

        if (strncmp(p, "char", 4) == 0 && !is_alnum(p[4])) {
            tokens_push(TK_CHAR, line, column);
            p += 4;
            continue;
        }

        if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
            tokens_push(TK_FOR, line, column);
            p += 3;
            continue;
        }

        if (strncmp(p, "int", 3) == 0 && !is_alnum(p[3])) {
            tokens_push(TK_INT, line, column);
            p += 3;
            continue;
        }

        if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
            tokens_push(TK_IF, line, column);
            p += 2;
            continue;
        }

        if (!strncmp(p, "<=", 2)) {
            tokens_push(TK_LESS_EQUAL, line, column);
            p += 2;
            continue;
        }

        if (!strncmp(p, ">=", 2)) {
            tokens_push(TK_MORE_EQUAL, line, column);
            p += 2;
            continue;
        }

        if (!strncmp(p, "==", 2)) {
            tokens_push(TK_EQUAL_EQUAL, line, column);
            p += 2;
            continue;
        }

        if (!strncmp(p, "!=", 2)) {
            tokens_push(TK_EXCL_EQUAL, line, column);
            p += 2;
            continue;
        }

        if (!strncmp(p, "+=", 2)) {
            tokens_push(TK_PLUS_EQUAL, line, column);
            p += 2;
            continue;
        }

        if (!strncmp(p, "++", 2)) {
            tokens_push(TK_PLUS_PLUS, line, column);
            p += 2;
            continue;
        }

        switch (*p) {
            case '+':
                tokens_push(TK_PLUS, line, column);
                p++;
                continue;
            case '-':
                tokens_push(TK_MINUS, line, column);
                p++;
                continue;
            case '*':
                tokens_push(TK_ASTERISK, line, column);
                p++;
                continue;
            case '/':
                tokens_push(TK_SLASH, line, column);
                p++;
                continue;
            case '%':
                tokens_push(TK_PERCENT, line, column);
                p++;
                continue;
            case '(':
                tokens_push(TK_LEFT_PAREN, line, column);
                p++;
                continue;
            case ')':
                tokens_push(TK_RIGHT_PAREN, line, column);
                p++;
                continue;
            case '{':
                tokens_push(TK_LEFT_BRACE, line, column);
                p++;
                continue;
            case '}':
                tokens_push(TK_RIGHT_BRACE, line, column);
                p++;
                continue;
            case '[':
                tokens_push(TK_LEFT_SQ_BRACKET, line, column);
                p++;
                continue;
            case ']':
                tokens_push(TK_RIGHT_SQ_BRACKET, line, column);
                p++;
                continue;
            case '<':
                tokens_push(TK_LESS, line, column);
                p++;
                continue;
            case '>':
                tokens_push(TK_MORE, line, column);
                p++;
                continue;
            case '=':
                tokens_push(TK_EQUAL, line, column);
                p++;
                continue;
            case ';':
                tokens_push(TK_SEMICOLON, line, column);
                p++;
                continue;
            case ',':
                tokens_push(TK_COMMA, line, column);
                p++;
                continue;
            case '&':
                tokens_push(TK_AND, line, column);
                p++;
                continue;
        }

        if (*p == '"') {
            String str = new_string();
            while (*(++p) != '"') {
                string_push(str, *p);
            }
            tokens_push_string(str->buf, line, column);
            p++;
            continue;
        }

        if (*p == '\'') {
            p++;
            char c = *p;
            tokens_push_const_int(c, line, column);
            if (*(++p) != '\'')
                error_at(line, column, "文字定数が閉じられていません");
            p++;
            continue;
        }

        if (isdigit(*p)) {
            tokens_push_const_int(strtol(p, &p, 10), line, column);
            continue;
        }

        char *str;
        if ((str = identifier(&p)) != NULL) {
            tokens_push_ident(str, line, column);
            continue;
        }

        error_at(line, column, "トークナイズできません");
    }

    tokens_push(TK_EOF, line, column);
}
