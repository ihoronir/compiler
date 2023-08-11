#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdnoreturn.h>
#include <string.h>

// ポインタの可変長バッファ
typedef struct vec {
    void **buf;
    int len;
    int capacity;
} *Vec;

extern Vec string_storage;

// char の可変長バッファ
typedef struct string {
    char *buf;
    int len;
    int capacity;
} *String;

// トークンの種類
typedef enum {
    TK_RETURN,            // "return"
    TK_IF,                // "if"
    TK_ELSE,              // "else"
    TK_WHILE,             // "while"
    TK_FOR,               // "for"
    TK_INT,               // "int"
    TK_CHAR,              // "char"
    TK_CONST_INT,         // "10" などの整数
    TK_IDENT,             // "a" などの識別子
    TK_STRING,            // 文字列リテラル
    TK_SEMICOLON,         // ";"
    TK_COMMA,             // ","
    TK_EQUAL,             // "="
    TK_AND,               // "&"
    TK_LEFT_PAREN,        // "("
    TK_RIGHT_PAREN,       // ")"
    TK_LEFT_BRACE,        // "{"
    TK_RIGHT_BRACE,       // "}"
    TK_LEFT_SQ_BRACKET,   // "["
    TK_RIGHT_SQ_BRACKET,  // "]"
    TK_PLUS,              // "+"
    TK_MINUS,             // "-"
    TK_ASTERISK,          // "*"
    TK_SLASH,             // "/"
    TK_PERCENT,           // "%"
    TK_LESS,              // "<"
    TK_MORE,              // ">"
    TK_LESS_EQUAL,        // "<="
    TK_MORE_EQUAL,        // ">="
    TK_EQUAL_EQUAL,       // "=="
    TK_EXCL_EQUAL,        // "!="
    TK_PLUS_EQUAL,        // "+="
    TK_PLUS_PLUS,         // "++"
    TK_SIZEOF,            // "sizeof"
    TK_EOF,               // EOF
} TokenKind;

// トークン
typedef struct token {
    TokenKind kind;  // トークンの種類
    int val_int;     // kind が TK_CONST_INT の場合、その数値
    char *str;       // kind が TK_IDENT の場合、その文字列
    int line;        // トークンの行
    int column;      // トークンの列
} *Token;

typedef enum {
    EXP_CONST_INT,      // 定数
    EXP_LOCAL_VAR,      // ローカル変数
    EXP_GLOBAL_VAR,     //
    EXP_FUNC,           //
    EXP_STRING,         //
    EXP_DEREF,          // *[0]
    EXP_ADDR,           // &[0]
    EXP_MUL,            // [0] * [1]
    EXP_DIV,            // [0] / [1]
    EXP_MOD,            // [0] % [1]
    EXP_ADD,            // [0] + [1]
    EXP_SUB,            // [0] - [1]
    EXP_LESS,           // [0] < [1]
    EXP_LESS_OR_EQUAL,  // [0] <= [1]
    EXP_COMPOUND_ADD,   // [0] += [b]
    EXP_EQUAL,          // [0] == [1]
    EXP_NOT_EQUAL,      // [0] != [1]
    EXP_ASSIGN,         // [0] = [1]
    EXP_CALL,           // 関数呼び出し [0] ([1], [2], [3]),
    EXP_SIZEOF,         //
    EXP_DECAY,          //
} ExprKind;

typedef enum {
    STMT_RETURN,    // return [0]
    STMT_IF,        // if ([0]) [1]
    STMT_IF_ELSE,   // if ([0]) [1] else [2]
    STMT_WHILE,     // while ([0]) [1]
    STMT_FOR,       // for ([0]; [1]; [2]) [3]
    STMT_BLOCK,     // ブロック { [0] [1] [2] ...}
    STMT_ONLY_EXPR  //
} StmtKind;

typedef struct item *Item;

typedef struct string_item {
    int label;
    char *str;
} *StringItem;

StringItem new_string_item(char *str);

// 式
typedef struct untyped_expr {
    ExprKind kind;  // ノードの種類
    Vec children;   // 子要素
    // Token token;    // 対応するトークン
    Item item;
    int val_int;
    StringItem string_item;
} *UntypedExpr;

typedef enum {
    REG_RAX = 0,
    REG_RDI = 1,
    REG_RSI = 2,
    REG_RDX = 3,
    REG_RCX = 4,
    REG_RBP = 5,
    REG_RSP = 6,
    REG_RBX = 7,
    REG_R8 = 8,
    REG_R9 = 9,
    REG_R10 = 10,
    REG_R11 = 11,
    REG_R12 = 12,
    REG_R13 = 13,
    REG_R14 = 14,
    REG_R15 = 15
} RegKind;

// 型の種類
typedef enum { TY_INT, TY_CHAR, TY_FUNC, TY_ARR, TY_PTR } TypeKind;

// 型の型
typedef struct type {
    TypeKind kind;
    struct type *ptr_to;
    struct type *returning;
    int arr_len;
} *Type;

typedef struct typed_expr {
    ExprKind kind;  // ノードの種類
    Vec children;   // 子要素
    // Token token;    // 対応するトークン
    Item item;
    Type type;
    StringItem string_item;
    int val_int;
} *TypedExpr;

// 文
typedef struct stmt {
    StmtKind kind;              // ノードの種類
    Vec stmt_children;          // 子要素
    Vec untyped_expr_children;  //
    Vec typed_expr_children;    //
    // Token token;    // 対応するトークン
    // Expr typed_expr;
    Item item;  // kind が ND_LOCAL_VAR, ND_CALL, ND_FUNC の場合、そのアイテム
    int val_int;  // kind が ND_CONST_INT の場合、その数値
} *Stmt;

typedef enum { TLD_FUNC_DEF, TLD_GLOBAL_VAR_DEF } ToplevelDefinitionKind;

// トップレベル定義
typedef struct toplevel_definition {
    ToplevelDefinitionKind kind;  // ノードの種類
    Vec stmt_children;            // 子要素
    Vec untyped_expr_children;    //
    Vec typed_expr_children;      //
    Item item;
} *ToplevelDefinition;

// スコープの型
typedef struct scope {
    struct scope *parent;  // 親スコープ
    Vec items;             // スコープに登録されているアイテム
    Item func;  // ルートスコープでない場合、属する関数
} *Scope;

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
void error(char *fmt, ...) __attribute__((__noreturn__));
void error_at(int line, int column, char *msg);
void *checked_malloc(unsigned long len);
void *checked_realloc(void *ptr, unsigned long len);

// token.c
Token new_token(TokenKind kind, int line, int column);
Token new_token_const_int(int val, int line, int column);
Token new_token_ident(char *str, int line, int column);
Token new_token_string(char *str, int line, int column);

// tokens.c
void tokens_push(TokenKind kind, int line, int column);
void tokens_push_const_int(int val, int line, int column);
void tokens_push_ident(char *str, int line, int column);
void tokens_push_string(char *str, int line, int column);
Token tokens_next();
Token tokens_peek();

// tokenize.c
void tokenize(char *p);

// type.c
int type_size(Type type);
char *type_reg(Type type);
Type new_type_int();
Type new_type_char();
Type new_type_arr(Type arr_of, int arr_len);
Type new_type_ptr(Type ptr_to);
Type new_type_func(Type returning);
int type_is_equal(Type type1, Type type2);
int type_is_compatible(Type type1, Type type2);
int type_is_integer(Type type);
char *type_reg_name(RegKind reg_kind, Type type);
char *type_mov_cmd(Type type);

// scope.c
Scope new_scope_global();
Scope new_scope_func(Scope parent, Item item);
Scope new_scope(Scope parent);
Item scope_get_item(Scope scope, char *name);
Item scope_def_func(Scope scope, Type type, char *name);
Item scope_def_local_var(Scope scope, Type type, char *name);
Item scope_def_global_var(Scope scope, Type type, char *name);

// item.c
Item new_item_local_var(Type type, char *name, int offset);
Item new_item_global_var(Type type, char *name);
Item new_item_func(Type type, char *name);

// expr.c
UntypedExpr new_untyped_expr(ExprKind kind, ...);
UntypedExpr new_untyped_expr_const_int(int val_int);
UntypedExpr new_untyped_expr_string(StringItem string_item);
// UntypedExpr new_untyped_expr_local_var(Scope scope, char *name);
UntypedExpr new_untyped_expr_local_var(Item item);
UntypedExpr new_untyped_expr_local_var_or_func_or_global_var(Scope scope,
                                                             char *name);
UntypedExpr new_untyped_expr_local_var_with_def(Scope scope, Type type,
                                                char *name);
UntypedExpr new_untyped_expr_call(Vec children);
UntypedExpr untyped_expr_get_child(UntypedExpr untyped_expr, int index);

ToplevelDefinition new_toplevel_definition_func(Scope scope, Type type,
                                                char *name,
                                                Vec untyped_expr_children,
                                                Vec stmt_children);
ToplevelDefinition new_toplevel_definition_global_var(Scope scope, Type type,
                                                      char *name);

// Stmt new_stmt(StmtKind kind, ...);
Stmt new_stmt_only_expr(UntypedExpr untyped_expr);
Stmt new_stmt_return(UntypedExpr untyped_expr);
Stmt new_stmt_func_definition(Scope scope, Type type, char *name,
                              Vec untyped_expr_children, Vec stmt_children);
Stmt new_stmt_block(Vec stmt_children);

Stmt new_stmt_if(UntypedExpr cond, Stmt then_stmt);
Stmt new_stmt_if_else(UntypedExpr cond, Stmt then_stmt, Stmt else_stmt);
Stmt new_stmt_while(UntypedExpr cond, Stmt loop_stmt);
Stmt new_stmt_for(UntypedExpr init, UntypedExpr cond, UntypedExpr update,
                  Stmt loop_stmt);

Stmt stmt_get_stmt_child(Stmt stmt, int index);
UntypedExpr stmt_get_untyped_expr_child(Stmt stmt, int index);
TypedExpr stmt_get_typed_expr_child(Stmt stmt, int index);

// expr
TypedExpr to_typed_expr(UntypedExpr ue);
TypedExpr typed_expr_get_child(TypedExpr typed_expr, int index);

// parse.c
Vec /* <ToplevelDefinition> */ parse_program();

// check.c
void make_stmt_typed(Stmt stmt);

// gen.c
void gen_program(Vec /* <ToplevelDefinition> */ program, FILE *out_fp);

// vec.c
Vec new_vec_with_capacity(int capacity);
Vec new_vec();
void vec_push(Vec vec, void *ptr);

// string.c
String new_string_with_capacity(int capacity);
String new_string();
void string_push(String string, char c);
