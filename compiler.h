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
    // TK_RESERVED,  // 記号
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

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind;  // トークンの型
    int val;         // kindがTK_INTの場合、その数値
    char *str;       // トークン文字列
    int line;        // トークンの行
    int row;         // トークンの列
};

Token *token_next();
Token *token_peek();
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
    ND_ASSGIN,         // =
    ND_RETURN          // return
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺・返り値
    Node *rhs;      // 右辺
    int val;        // kindがND_CONSTの場合のみ使う
    int offset;     // kindがND_ASSGINの場合のみ使う
};

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

void gen(Node *node);
