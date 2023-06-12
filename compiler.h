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

    TK_NUM,

    TK_IDENTIFIER,

    TK_LEFT_PARENTHESES,
    TK_RIGHT_PARENTHESES,

    TK_PLUS,
    TK_MINUS,
    TK_ASTERISK,
    TK_SLASH,

    TK_LESS,
    TK_MORE,

    TK_LESS_OR_EQUAL,
    TK_MORE_OR_EQUAL,

    TK_EQUAL,
    TK_NOT_EQUAL,

    TK_EOF,  // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

// トークン型
struct Token {
    TokenKind kind;  // トークンの型
    int val;         // kindがTK_NUMの場合、その数値
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
    ND_NUM,    // 整数
    ND_LVAR,   // ローカル変数
    ND_MUL,    // *
    ND_DIV,    // /
    ND_ADD,    // +
    ND_SUB,    // -
    ND_LT,     // <
    ND_LE,     // <=
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_ASSGIN  // =
} NodeKind;

typedef struct Node Node;

// 抽象構文木のノードの型
struct Node {
    NodeKind kind;  // ノードの型
    Node *lhs;      // 左辺
    Node *rhs;      // 右辺
    int val;        // kindがND_NUMの場合のみ使う
    int offset;     // kindがND_ASSGINの場合のみ使う
};

void ast_init();
void ast_compile();

///
/// gen.c
///

void gen(Node *node);
