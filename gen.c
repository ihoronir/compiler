#include "compiler.h"

static int gen_id() {
    static int id = 0;
    return id++;
}

static void print(int indent, char *fmt, ...) {
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
    print(indent, "# gen_address");
    print(indent + 1, "mov rax, rbp");
    print(indent + 1, "sub rax, %d", node->item->offset);
    print(indent + 1, "push rax");
}

void gen(Node node, int indent) {
    int id;

    switch (node->kind) {
        case ND_PROGRAM:
            print(indent, "# ND_PROGRAM");
            print(indent + 1, ".intel_syntax noprefix");
            print(indent + 1, ".globl main");

            for (int i = 0; i < node->children->len; i++) {
                gen(node_get_child(node, i), indent + 1);
            }
            break;

        case ND_FUNC:
            print(indent, "# ND_FUNC");
            print(indent + 1, "%s:", node->item->name);
            print(indent + 2, "push rbp");
            print(indent + 2, "mov rbp, rsp");
            print(indent + 2, "sub rsp, %d", node->item->size);

            if (node->children->len > 7)
                error("6 個以上の仮引数には対応していません");

            for (int i = 0; i < node->children->len - 1; i++) {
                gen_address(node_get_child(node, i), indent + 2);
                print(indent + 2, "pop rax");

                switch (i) {
                    case 0:
                        print(indent + 2, "mov [rax], rdi");
                        break;
                    case 1:
                        print(indent + 2, "mov [rax], rsi");
                        break;
                    case 2:
                        print(indent + 2, "mov [rax], rdx");
                        break;
                    case 3:
                        print(indent + 2, "mov [rax], rcx");
                        break;
                    case 4:
                        print(indent + 2, "mov [rax], r8");
                        break;
                    case 5:
                        print(indent + 2, "mov [rax], r9");
                        break;
                }
            }

            gen(node_get_child(node, node->children->len - 1), indent + 2);

            print(indent + 2, "pop rax");
            print(indent + 2, "mov rsp, rbp");
            print(indent + 2, "pop rbp");
            print(indent + 2, "ret");
            break;

        case ND_CALL:
            print(indent, "# ND_CALL");

            if (node->children->len > 6)
                error("6 個以上の実引数には対応していません");

            for (int i = node->children->len - 1; i >= 0; i--) {
                gen(node_get_child(node, i), indent + 1);

                switch (i) {
                    case 0:
                        print(indent + 1, "pop rdi");
                        break;
                    case 1:
                        print(indent + 1, "pop rsi");
                        break;
                    case 2:
                        print(indent + 1, "pop rdx");
                        break;
                    case 3:
                        print(indent + 1, "pop rcx");
                        break;
                    case 4:
                        print(indent + 1, "pop r8");
                        break;
                    case 5:
                        print(indent + 1, "pop r9");
                        break;
                }
            }

            print(indent + 1, "push %d", node->children->len);
            print(indent + 1, "pop rax");
            print(indent + 1, "call %s", node->item->name);
            print(indent + 1, "push rax");
            break;

        case ND_BLOCK:
            print(indent, "# ND_BLOCK");

            for (int i = 0; i < node->children->len; i++) {
                gen(node_get_child(node, i), indent + 1);
                print(indent + 1, "pop rax");
            }
            print(indent + 1, "push rax");
            break;

        case ND_RETURN:
            print(indent, "# ND_RETURN");

            gen(node_get_child(node, 0), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "mov rsp, rbp");
            print(indent + 1, "pop rbp");
            print(indent + 1, "ret");
            break;

        case ND_CONST_INT:
            print(indent, "# ND_CONST_INT");

            print(indent + 1, "push %d", node->val_int);
            break;

        case ND_LOCAL_VAR:
            print(indent, "# ND_LOCAL_VAR");

            gen_address(node, indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "mov rax, [rax]");
            print(indent + 1, "push rax");
            break;

        case ND_ASSIGN:
            print(indent, "# ND_ASSIGN");

            gen_address(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");
            print(indent + 1, "mov [rax], rdi");
            print(indent + 1, "push rdi");
            break;

        case ND_NULL:
            print(indent, "# ND_NULL");
            print(indent + 1, "push rax");
            break;

        case ND_IF:
            id = gen_id();

            print(indent, "# ND_IF");

            gen(node_get_child(node, 0), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "cmp rax, 0");
            print(indent + 1, "je .Lend%08d", id);

            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, ".Lend%08d:", id);
            print(indent + 1, "push rax");
            break;

        case ND_IF_ELSE:
            id = gen_id();

            print(indent, "# ND_IF_ELSE");

            gen(node_get_child(node, 0), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "cmp rax, 0");
            print(indent + 1, "je .Lelse%08d", id);

            gen(node_get_child(node, 1), indent + 1);

            // print(indent + 1, "pop rax");
            print(indent + 1, "jmp .Lend%08d", id);
            print(indent + 1, ".Lelse%08d:", id);

            gen(node_get_child(node, 2), indent + 1);

            // print(indent + 1, "pop rax");
            print(indent + 1, ".Lend%08d:", id);
            // print(indent + 1, "push rax");

            break;

        case ND_WHILE:
            id = gen_id();

            print(indent, "# ND_WHILE");

            print(indent + 1, ".Lbegin%08d:", id);

            gen(node_get_child(node, 0), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "cmp rax, 0");
            print(indent + 1, "je .Lend%08d", id);

            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "jmp .Lbegin%08d", id);
            print(indent + 1, ".Lend%08d:", id);
            print(indent + 1, "push rax");

            break;

        case ND_FOR:
            id = gen_id();

            print(indent, "# ND_FOR");

            gen(node_get_child(node, 0), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, ".Lbegin%08d:", id);

            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "cmp rax, 0");
            print(indent + 1, "je .Lend%08d", id);

            gen(node_get_child(node, 3), indent + 1);

            print(indent + 1, "pop rax");

            gen(node_get_child(node, 2), indent + 1);

            print(indent + 1, "pop rax");
            print(indent + 1, "jmp .Lbegin%08d", id);
            print(indent + 1, ".Lend%08d:", id);
            print(indent + 1, "push rax");

            break;

        case ND_ADD:
            print(indent, "# ND_ADD");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "add rax, rdi");
            print(indent + 1, "push rax");
            break;

        case ND_SUB:
            print(indent, "# ND_SUB");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "sub rax, rdi");
            print(indent + 1, "push rax");
            break;

        case ND_MUL:
            print(indent, "# ND_MUL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "imul rax, rdi");
            print(indent + 1, "push rax");
            break;

        case ND_DIV:
            print(indent, "# ND_DIV");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "cqo");
            print(indent + 1, "idiv rdi");
            print(indent + 1, "push rax");
            break;

        case ND_EQUAL:
            print(indent, "# ND_EQUAL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "cmp rax, rdi");
            print(indent + 1, "sete al");
            print(indent + 1, "movzb rax, al");
            print(indent + 1, "push rax");
            break;

        case ND_NOT_EQUAL:
            print(indent, "# ND_NOT_EQUAL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "cmp rax, rdi");
            print(indent + 1, "setne al");
            print(indent + 1, "movzb rax, al");
            print(indent + 1, "push rax");
            break;

        case ND_LESS:
            print(indent, "# ND_LESS");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "cmp rax, rdi");
            print(indent + 1, "setl al");
            print(indent + 1, "movzb rax, al");
            print(indent + 1, "push rax");
            break;

        case ND_LESS_OR_EQUAL:
            print(indent, "# ND_LESS_OR_EQUAL");

            gen(node_get_child(node, 0), indent + 1);
            gen(node_get_child(node, 1), indent + 1);

            print(indent + 1, "pop rdi");
            print(indent + 1, "pop rax");

            print(indent + 1, "cmp rax, rdi");
            print(indent + 1, "setle al");
            print(indent + 1, "movzb rax, al");
            print(indent + 1, "push rax");
            break;
    }
}
