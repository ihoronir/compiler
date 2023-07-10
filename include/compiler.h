#include <ctype.h>
#include <errno.h>
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
    TK_INT,          // "int"
    TK_CONST_INT,    // "10" などの整数
    TK_IDENT,        // "a" などの識別子
    TK_SEMICOLON,    // ";"
    TK_COMMA,        // ","
    TK_EQUAL,        // "="
    TK_AND,          // "&"
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
    int val_int;     // kind が TK_CONST_INT の場合、その数値
    char *str;       // kind が TK_IDENT の場合、その文字列
    int line;        // トークンの行
    int row;         // トークンの列
} *Token;

// ノードの種類
typedef enum {
    ND_CONST_INT,      // 定数
    ND_LOCAL_VAR,      // ローカル変数
    ND_DEREF,          // *[0]
    ND_ADDR,           // &[0]
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
    ND_FOR,            // for ([0]; [1]; [2]) [3]
    ND_NULL,           // 空文
    ND_BLOCK,          // ブロック { [0] [1] [2] ...}
    ND_FUNC,  // 関数定義 func_name([0], [1], [2], [3], [4], ..., [n-1]) [n]
    ND_CALL,  // 関数呼び出し func_name([0], [1], [2], [3], [4], ...)
    ND_PROGRAM  // プログラム全体 [0] [1] [2] ...
} NodeKind;

typedef struct item *Item;

// ノードの型
typedef struct node {
    NodeKind kind;  // ノードの種類
    Vec children;   // 子要素
    // Token token;    // 対応するトークン
    Item item;  // kind が ND_LOCAL_VAR, ND_CALL, ND_FUNC の場合、そのアイテム
    int val_int;  // kind が ND_CONST_INT の場合、その数値
} *Node;

// スコープの型
typedef struct scope {
    struct scope *parent;  // 親スコープ
    Vec items;             // スコープに登録されているアイテム
    Item func;  // ルートスコープでない場合、属する関数
} *Scope;

// 型の種類
typedef enum { TY_INT, TY_FUNC, TY_ARR, TY_PTR } TypeKind;

// 型の型
typedef struct type {
    TypeKind kind;
    struct type *ptr_to;
    struct type *returning;
    int arr_len;
} *Type;

// アイテムの種類
typedef enum {
    IT_FUNC,
    IT_TYPEDEF,
    IT_GLOBAL_VAR,
    IT_LOCAL_VAR,
} ItemKind;

// アイテムの型
struct item {
    ItemKind kind;  // アイテムの種類
    Type type;  // kind が IT_LOCAL_VAR または IT_GLOBAL_VAR の場合、その型
    char *name;  // アイテムの名前
    int size;    // kind がIT_FUNC の場合、そのサイズ
    int offset;  // kind が IT_LOCAL_VAR の場合、そのオフセット
};

// main.c
void error(char *fmt, ...);
void error_at(int line, int row, char *msg);
void *checked_malloc(unsigned long len);
void *checked_realloc(void *ptr, unsigned long len);

// token.c
Token new_token(TokenKind kind, int line, int row);
Token new_token_const_int(int val, int line, int row);
Token new_token_ident(char *str, int line, int row);

// tokens.c
void tokens_push(TokenKind kind, int line, int row);
void tokens_push_const_int(int val, int line, int row);
void tokens_push_ident(char *str, int line, int row);
Token tokens_next();
Token tokens_peek();

// tokenize.c
void tokenize(char *p);

// type.c
Type new_type_int();
Type new_type_ptr(Type ptr_to);
Type new_type_func(Type returning);

// scope.c
Scope new_scope_global();
Scope new_scope_func(Scope parent, Item item);
Scope new_scope(Scope parent);
Item scope_get_item(ItemKind kind, Scope scope, char *name);
Item scope_def_func(Scope scope, Type type, char *name);
Item scope_def_local_var(Scope scope, Type type, char *name);

// item.c
Item new_item_local_var(Type type, char *name, int offset);
Item new_item_func(Type type, char *name);

// node.c
Node new_node(NodeKind kind, ...);
Node new_node_const_int(int val);
Node new_node_local_var(Scope scope, char *name);
Node new_node_local_var_with_def(Scope scope, Type type, char *name);
Node new_node_func(Scope scope, Type type, char *name, Vec children);
Node new_node_null();
Node new_node_block(Vec childs);
Node new_node_call(Scope scope, char *name, Vec children);
Node new_node_program(Vec children);
Node node_get_child(Node node, int index);

// parse.c
Node program();

// check.c
void check(Node node);

// gen.c
void gen_program(Node node, FILE *fp);

// vec.c
Vec new_vec_with_capacity(int capacity);
Vec new_vec();
void vec_push(Vec vec, void *ptr);

// string.c
String new_string_with_capacity(int capacity);
String new_string();
void string_push(String string, char c);
