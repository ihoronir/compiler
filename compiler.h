#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// ポインタの可変長バッファ
typedef struct vec {
    void **buf;
    int len;
    int capacity;
} *Vec;

// トークンの種類
typedef enum {
    TK_RETURN,       // "return"
    TK_IF,           // "if"
    TK_ELSE,         // "else"
    TK_WHILE,        // "while"
    TK_FOR,          // "for"
    TK_INT,          // "10" などの整数
    TK_IDENT,        // "a" などの識別子
    TK_SEMICOLON,    // ";"
    TK_EQUAL,        // "="
    TK_LEFT_PAREN,   // "("
    TK_RIGHT_PAREN,  // ")"
    TK_LEFT_BRACE,   // "{"
    TK_RIGHT_BRACE,  // "}"
    TK_PLUS,         // "+"
    TK_MINUS,        // "-"
    TK_ASTERISK,     // "*"
    TK_SLASH,        // "/"
    TK_LESS,         // "<"
    TK_MORE,         // ">"
    TK_LESS_EQUAL,   // "<="
    TK_MORE_EQUAL,   // ">="
    TK_EQUAL_EQUAL,  // "=="
    TK_EXCL_EQUAL,   // "!="
    TK_EOF,          // EOF
} TokenKind;

// トークン
typedef struct token {
    TokenKind kind;  // トークンの種類
    int val;         // kind が TK_INT の場合、その数値
    char *str;       // kind が TK_IDENT の場合、その文字列
    int line;        // トークンの行
    int row;         // トークンの列
} *Token;

// AST のノードの種類
typedef enum {
    ND_CONST,          // 定数
    ND_LOCAL_VAR,      // ローカル変数
    ND_MUL,            // childs[0] * childs[1]
    ND_DIV,            // childs[0] / childs[1]
    ND_ADD,            // childs[0] + childs[1]
    ND_SUB,            // childs[0] - childs[1]
    ND_LESS,           // childs[0] < childs[1]
    ND_LESS_OR_EQUAL,  // childs[0] <= childs[1]
    ND_EQUAL,          // childs[0] == childs[1]
    ND_NOT_EQUAL,      // childs[0] != childs[1]
    ND_ASSIGN,         // childs[0] = childs[1]
    ND_RETURN,         // return childs[0]
    ND_IF,             // if (childs[0]) childs[1]
    ND_IF_ELSE,        // if (childs[0]) childs[1] else childs[2]
    ND_WHILE,          // while (childs[0]) childs[1]
    ND_FOR,            // for (childs[0], childs[1], childs[2]) childs[3]
    ND_NULL,           // 空文
    ND_BLOCK,          // ブロック
} NodeKind;

// AST のノードの型
typedef struct node {
    NodeKind kind;  // ノードの種類
    Vec childs;     // 子要素
    int offset;     // kind が ND_LOCAL_VAR の場合、そのオフセット
    int val;        // kind が ND_CONST の場合、その数値
                    /*
                    struct Node *lhs;      // 左辺
                    struct Node *rhs;      // 右辺
                    struct Node *init;     // for の初期化式
                    struct Node *cond;     // if, while, for の条件式
                    struct Node *update;   // for の更新式
                    struct Node *then;     // if, while, for で条件式が成立したとき
                    struct Node *alt;      // if で条件式が不成立のとき
                    struct Node *program;  // stmt のベクタ
                    */
} *Node;

// main.c
void error(char *msg);
void error_at(int line, int row, char *msg);
void *checkd_malloc(unsigned long len);
void *checkd_realloc(void *ptr, unsigned long len);

// token.c
Token new_token(TokenKind tk, int line, int row);
Token new_token_int(int val, int line, int row);
Token new_token_ident(char *str, int line, int row);

// tokens.c
void tokens_push(TokenKind tk, int line, int row);
void tokens_push_int(int val, int line, int row);
void tokens_push_ident(char *str, int line, int row);
Token tokens_next();
Token tokens_peek();

// tokenize.c
void tokenize(char *p);

// node.c
Node new_node(NodeKind nk, ...);
Node new_node_block(Vec childs);
Node new_node_const(int val);
Node new_node_local_var(int offset);
Node new_node_null();
Node node_get_child(Node node, int index);

// parse.c
Node program();

// local_vars.c
void init_local_vars_buf();
int get_offset(char *str);

// gen.c
void gen(Node node, int indent);

// vec.c
Vec new_vec_with_capacity(int capacity);
Vec new_vec();
void vec_push(Vec vec, void *ptr);
