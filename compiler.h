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

// char の可変長バッファ
typedef struct string {
    char *buf;
    int len;
    int capacity;
} *String;

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
    TK_COMMA,        // ","
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
    ND_MUL,            // [0] * [1]
    ND_DIV,            // [0] / [1]
    ND_ADD,            // [0] + [1]
    ND_SUB,            // [0] - [1]
    ND_LESS,           // [0] < [1]
    ND_LESS_OR_EQUAL,  // [0] <= [1]
    ND_EQUAL,          // [0] == [1]
    ND_NOT_EQUAL,      // [0] != [1]
    ND_ASSIGN,         // [0] = [1]
    ND_RETURN,         // return [0]
    ND_IF,             // if ([0]) [1]
    ND_IF_ELSE,        // if ([0]) [1] else [2]
    ND_WHILE,          // while ([0]) [1]
    ND_FOR,            // for ([0], [1], [2]) [3]
    ND_NULL,           // 空文
    ND_BLOCK,          // ブロック { [0] [1] [2] ...}
    ND_FUNC,  // 関数定義 func_name(..., [6], [5], [4], [3], [2], [1]) { [0] }
    ND_CALL,  // 関数呼び出し func_name(..., [5], [4], [3], [2], [1], [0])
    ND_PROGRAM  // プログラム全体 [0] [1] [2] ...
} NodeKind;

// AST のノードの型
typedef struct node {
    NodeKind kind;  // ノードの種類
    Vec children;   // 子要素
    int offset;  // kind が ND_LOCAL_VAR または ND_ARG の場合、そのオフセット
    int val;     // kind が ND_CONST の場合、その数値
    char *name;  // kind が ND_FUNC の場合、関数名
    int size;    // kind が ND_FUNC の場合、サイズ
} *Node;

// ネームスペースの型
typedef struct name_space {
    Vec items;
    struct name_space *parent;
    int size;  // ネームスペースおよびその子孫のネームスペースに
               // 含まれるローカル変数の、最大 offset
} *NameSpace;

// アイテムの種類
typedef enum {
    IT_FUNC,
    IT_LOCAL_VAR,
} ItemKind;

// アイテムの型
typedef struct item {
    ItemKind kind;  // アイテムの種類
    char *name;     // アイテムの名前
    int offset;     // kind が LocalVar の場合、そのオフセット
} *Item;

// name_space.c
NameSpace new_name_space(NameSpace parent);
void name_space_def_func(NameSpace name_space, char *name);
void name_space_def_local_var(NameSpace name_space, char *name);
int name_space_get_local_var_offset(NameSpace name_space, char *name);

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
Node new_node_const(int val);
Node new_node_local_var(int offset);
Node new_node_null();
Node node_get_child(Node node, int index);
Node new_node_block(Vec childs);
Node new_node_func(char *name, int size, Vec children);
Node new_node_call(char *name, Vec children);
Node new_node_program(Vec children);

// parse.c
Node program();

// gen.c
void gen(Node node, int indent);

// vec.c
Vec new_vec_with_capacity(int capacity);
Vec new_vec();
void vec_push(Vec vec, void *ptr);

// string.c
String new_string_with_capacity(int capacity);
String new_string();
void string_push(String string, char c);
