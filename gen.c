#include "compiler.h"

static int gen_id() {
    static int id = 0;
    return id++;
}

static void print_with_indent(int indent, char *fmt, ...) {
    for (int i = 0; i < indent; i++) {
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
    print_with_indent(indent + 1, "sub rax, %d", node->item->offset);
    print_with_indent(indent + 1, "push rax");
}

void gen(Node node, int indent) {
    int id;

    switch (node->kind) {
        case ND_PROGRAM:
            print_with_indent(indent, "# ND_PROGRAM");
            print_with_indent(indent + 1, ".intel_syntax noprefix");
            print_with_indent(indent + 1, ".globl main");

            for (int i = 0; i < node->children->len; i++) {
                gen(node_get_child(node, i), indent + 1);
            }
            break;

        case ND_FUNC:
            print_with_indent(indent, "# ND_FUNC");
            print_with_indent(indent + 1, "%s:", node->item->name);
            print_with_indent(indent + 2, "push rbp");
            print_with_indent(indent + 2, "mov rbp, rsp");
            print_with_indent(indent + 2, "sub rsp, %d", node->item->size);

            if (node->children->len > 7)
                error("6 個以上の仮引数には対応していません");

            for (int i = 0; i < node->children->len - 1; i++) {
                gen_address(node_get_child(node, i), indent + 2);
                print_with_indent(indent + 2, "pop rax");

                switch (i) {
                    case 0:
                        print_with_indent(indent + 2, "mov [rax], rdi");
                        break;
                    case 1:
                        print_with_indent(indent + 2, "mov [rax], rsi");
                        break;
                    case 2:
                        print_with_indent(indent + 2, "mov [rax], rdx");
                        break;
                    case 3:
                        print_with_indent(indent + 2, "mov [rax], rcx");
                        break;
                    case 4:
                        print_with_indent(indent + 2, "mov [rax], r8");
                        break;
                    case 5:
                        print_with_indent(indent + 2, "mov [rax], r9");
                        break;
                }
            }

            gen(node_get_child(node, node->children->len - 1), indent + 2);

            print_with_indent(indent + 2, "pop rax");
            print_with_indent(indent + 2, "mov rsp, rbp");
            print_with_indent(indent + 2, "pop rbp");
            print_with_indent(indent + 2, "ret");
            break;

        case ND_CALL:
            print_with_indent(indent, "# ND_CALL");

            if (node->children->len > 6)
                error("6 個以上の実引数には対応していません");

            for (int i = node->children->len - 1; i >= 0; i--) {
                gen(node_get_child(node, i), indent + 1);

                switch (i) {
                    case 0:
                        print_with_indent(indent + 1, "pop rdi");
                        break;
                    case 1:
                        print_with_indent(indent + 1, "pop rsi");
                        break;
                    case 2:
                        print_with_indent(indent + 1, "pop rdx");
                        break;
                    case 3:
                        print_with_indent(indent + 1, "pop rcx");
                        break;
                    case 4:
                        print_with_indent(indent + 1, "pop r8");
                        break;
                    case 5:
                        print_with_indent(indent + 1, "pop r9");
                        break;
                }
            }

            print_with_indent(indent + 1, "push %d", node->children->len);
            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "call %s", node->item->name);
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_BLOCK:
            print_with_indent(indent, "# ND_BLOCK");

            for (int i = 0; i < node->children->len; i++) {
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

        case ND_CONST_INT:
            print_with_indent(indent, "# ND_CONST_INT");

            print_with_indent(indent + 1, "push %d", node->val_int);
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

        case ND_IF:
            id = gen_id();

            print_with_indent(indent, "# ND_IF");

            gen(node_get_child(node, 0), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "cmp rax, 0");
            print_with_indent(indent + 1, "je .Lend%08d", id);

            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, ".Lend%08d:", id);
            print_with_indent(indent + 1, "push rax");
            break;

        case ND_IF_ELSE:
            id = gen_id();

            print_with_indent(indent, "# ND_IF_ELSE");

            gen(node_get_child(node, 0), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "cmp rax, 0");
            print_with_indent(indent + 1, "je .Lelse%08d", id);

            gen(node_get_child(node, 1), indent + 1);

            // print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "jmp .Lend%08d", id);
            print_with_indent(indent + 1, ".Lelse%08d:", id);

            gen(node_get_child(node, 2), indent + 1);

            // print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, ".Lend%08d:", id);
            // print_with_indent(indent + 1, "push rax");

            break;

        case ND_WHILE:
            id = gen_id();

            print_with_indent(indent, "# ND_WHILE");

            print_with_indent(indent + 1, ".Lbegin%08d:", id);

            gen(node_get_child(node, 0), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "cmp rax, 0");
            print_with_indent(indent + 1, "je .Lend%08d", id);

            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "jmp .Lbegin%08d", id);
            print_with_indent(indent + 1, ".Lend%08d:", id);
            print_with_indent(indent + 1, "push rax");

            break;

        case ND_FOR:
            id = gen_id();

            print_with_indent(indent, "# ND_FOR");

            gen(node_get_child(node, 0), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, ".Lbegin%08d:", id);

            gen(node_get_child(node, 1), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "cmp rax, 0");
            print_with_indent(indent + 1, "je .Lend%08d", id);

            gen(node_get_child(node, 3), indent + 1);

            print_with_indent(indent + 1, "pop rax");

            gen(node_get_child(node, 2), indent + 1);

            print_with_indent(indent + 1, "pop rax");
            print_with_indent(indent + 1, "jmp .Lbegin%08d", id);
            print_with_indent(indent + 1, ".Lend%08d:", id);
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
