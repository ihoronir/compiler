#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//
// main.c
//
void error(char *msg);
void error_at(int line, int row, char *msg);
void *checkd_malloc(unsigned long len);
void *checkd_realloc(void *ptr, unsigned long len);

//
// ts.c
//

// トークンの種類
typedef enum {
    TK_RETURN,

    TK_INT,

    TK_IDENT,

    TK_SEMICOLON,

    TK_EQUAL,

    TK_LEFT_PAREN,
    TK_RIGHT_PAREN,

    TK_PLUS,
    TK_MINUS,
    TK_ASTERISK,
    TK_SLASH,

    TK_LESS,
    TK_MORE,

    TK_LESS_EQUAL,
    TK_MORE_EQUAL,

    TK_EQUAL_EQUAL,
    TK_EXCL_EQUAL,

    TK_EOF,  // 入力の終わりを表すトークン
} TokenKind;

// トークン型
typedef struct Token {
    TokenKind kind;  // トークンの型
    int val;         // kindがTK_INTの場合、その数値
    char *str;       // トークン文字列
    int line;        // トークンの行
    int row;         // トークンの列
} Token;

void tokens_push(TokenKind tk, int line, int row);
void tokens_push_int(int val, int line, int row);
void tokens_push_ident(char *str, int line, int row);
Token *tokens_next();
Token *tokens_peek();

void tokenize(char *p);

//
// ast.c
//

// 抽象構文木のノードの種類
typedef enum {
    ND_CONST,          // 定数
    ND_LOCAL_VAR,      // ローカル変数
    ND_MUL,            // *
    ND_DIV,            // /
    ND_ADD,            // +
    ND_SUB,            // -
    ND_LESS,           // <
    ND_LESS_OR_EQUAL,  // <=
    ND_EQUAL,          // ==
    ND_NOT_EQUAL,      // !=
    ND_ASSIGN,         // =
    ND_RETURN          // return
} NodeKind;

// 抽象構文木のノードの型
typedef struct Node {
    NodeKind kind;     // ノードの型
    struct Node *lhs;  // 左辺・返り値
    struct Node *rhs;  // 右辺
    int val;           // kindがND_CONSTの場合のみ使う
    int offset;        // kindがND_ASSGINの場合のみ使う
} Node;

extern Node *code[100];

void program();

///
/// local_vars.c
///

void init_local_vars_buf();
int get_offset(char *str);

///
/// gen.c
///

void gen(Node *node, int indent);
