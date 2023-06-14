#include "compiler.h"

static void print_with_indent(int indent, char *fmt, ...) {
    int i;
    for (i = 0; i < indent; i++) {
        printf("    ");
    }
    va_list ap;
    va_start(ap, fmt);
    vprintf(fmt, ap);
    putchar('\n');
    va_end(ap);
}

static void gen_address(Node node, int indent) {
    if (node->kind != ND_LOCAL_VAR) error("代入の左辺値が変数ではありません");
    print_with_indent(indent, "# gen_address");
    print_with_indent(indent + 1, "mov rax, rbp");
    print_with_indent(indent + 1, "sub rax, %d", node->offset);
    print_with_indent(indent + 1, "push rax");
}

void gen(Node node, int indent) {
    int i;
    switch (node->kind) {
        case ND_PROGRAM:
            print_with_indent(indent, "# ND_PROGRAM");
            print_with_indent(indent + 1, ".intel_syntax noprefix");
            print_with_indent(indent + 1, ".globl main");

            for (i = 0; i < node->children->len; i++) {
                gen(node_get_child(node, i), indent + 1);
            }
            break;

        case ND_FUNC:
            print_with_indent(indent, "# ND_FUNC");
            print_with_indent(indent + 1, "%s:", node->name);
            print_with_indent(indent + 1, "push rbp");
            print_with_indent(indent + 1, "mov rbp, rsp");
            print_with_indent(indent + 1, "sub rsp, %d", node->size);

            gen(node_get_child(node, 0), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "mov rsp, rbp");
            print_with_indent(indent + 1, "pop rbp");
            print_with_indent(indent + 1, "ret");
            break;

        case ND_BLOCK:
            print_with_indent(indent, "# ND_BLOCK");

            for (i = 0; i < node->children->len; i++) {
                gen(node_get_child(node, i), indent + 1);
                print_with_indent(indent + 1, "pop rax");
            }
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_RETURN:
            print_with_indent(indent, "# ND_RETURN");

            gen(node_get_child(node, 0), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "mov rsp, rbp");
            print_with_indent(indent + 1, "pop rbp");
            print_with_indent(indent + 1, "ret");
            break;

        case ND_CONST:
            print_with_indent(indent, "# ND_CONST");

            print_with_indent(indent + 1, "push %d", node->val);
            break;

        case ND_LOCAL_VAR:
            print_with_indent(indent, "# ND_LOCAL_VAR");

            gen_address(node, indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "mov rax, [rax]");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_ASSIGN:
            print_with_indent(indent, "# ND_ASSIGN");

            gen_address(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "mov [rax], rdi");
            print_with_indent(indent + 1, "push rdi");
            break;

        case ND_NULL:
            print_with_indent(indent, "# ND_NULL");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_ADD:
            print_with_indent(indent, "# ND_ADD");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "add rax, rdi");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_SUB:
            print_with_indent(indent, "# ND_SUB");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "sub rax, rdi");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_MUL:
            print_with_indent(indent, "# ND_MUL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "imul rax, rdi");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_DIV:
            print_with_indent(indent, "# ND_DIV");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "cqo");
            print_with_indent(indent + 1, "idiv rdi");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_EQUAL:
            print_with_indent(indent, "# ND_EQUAL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "cmp rax, rdi");
            print_with_indent(indent + 1, "sete al");
            print_with_indent(indent + 1, "movzb rax, al");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_NOT_EQUAL:
            print_with_indent(indent, "# ND_NOT_EQUAL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "cmp rax, rdi");
            print_with_indent(indent + 1, "setne al");
            print_with_indent(indent + 1, "movzb rax, al");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_LESS:
            print_with_indent(indent, "# ND_LESS");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "cmp rax, rdi");
            print_with_indent(indent + 1, "setl al");
            print_with_indent(indent + 1, "movzb rax, al");
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_LESS_OR_EQUAL:
            print_with_indent(indent, "# ND_LESS_OR_EQUAL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rdi");
            print_with_indent(indent + 1, "pop rax");

            print_with_indent(indent + 1, "cmp rax, rdi");
            print_with_indent(indent + 1, "setle al");
            print_with_indent(indent + 1, "movzb rax, al");
            print_with_indent(indent + 1, "push rax");
            break;
    }
}
