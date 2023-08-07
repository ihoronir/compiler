#include "compiler.h"

// TypedNode new_typed_node(TypedNodeKind kind, ) {
//     int n = children_num(kind);
//     if (n <= 0) error("new_node: kind が不正です");
//
//     UntypedNode node = checked_malloc(sizeof(*node));
//     node->kind = kind;
//     node->children = new_vec_with_capacity(n);
//
//     va_list ap;
//     va_start(ap, kind);
//
//     UntypedNode node_child;
//     while ((node_child = va_arg(ap, UntypedNode)) != NULL) {
//         vec_push(node->children, node_child);
//     }
//
//     va_end(ap);
//
//     if (node->children->len != n) error("new_node: 引数の数が不正です");
//     return node;
// }
//
//
TypedNode to_typed(UntypedNode untyped_node, Type type) {
    TypedNode typed_node = checked_malloc(sizeof(*typed_node));
    typed_node->kind = untyped_node->kind;
    typed_node->item = untyped_node->item;
    typed_node->val_int = untyped_node->val_int;
    typed_node->type = type;

    // NodeKind kind;
    // Item item;
    // Vec children;
    // int val_int;
    // Type type;  // kind が IT_LOCAL_VAR または IT_GLOBAL_VAR の場合、その型
    return typed_node;
}

// TypedNode new_typed_node_const_int(int val_int) {
// }
//
// TypedNode new_typed_node_local_var(Item item) {
// }
//
////
//// UntypedNode new_node_null() {
////    UntypedNode node = checked_malloc(sizeof(*node));
////    node->kind = ND_NULL;
////    return node;
////}
////
// UntypedNode new_node_block(Vec children) {
//     UntypedNode typed_node = checked_malloc(sizeof(*typed_node));
//     typed_node->kind = ND_BLOCK;
//     typed_node->children = children;
//     return typed_node;
// }
////
//// UntypedNode new_node_call(Scope scope, char *name, Vec children) {
////    UntypedNode node = checked_malloc(sizeof(*node));
////    node->kind = ND_CALL;
////    node->children = children;
////    node->item = scope_get_item(IT_FUNC, scope, name);
////    return node;
////}
////
// TypedNode new_typed_node_program(Vec children) {
// }
////
//// UntypedNode node_get_child(UntypedNode node, int index) {
////    if (index >= node->children->len) error("node_get_child:
/// 範囲外アクセス"); /    return node->children->buf[index];
////}
