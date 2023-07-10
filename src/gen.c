#include "compiler.h"

static int gen_id() {
    static int id = 0;
    return id++;
}

static FILE *out;

static void print(int indent, char *fmt, ...) {
    for (int i = 0; i < indent; i++) {
        fprintf(out, " ");
    }
    va_list ap;
    va_start(ap, fmt);
    vfprintf(out, fmt, ap);
    fputc('\n', out);
    va_end(ap);
}

static void gen(Node node, int depth);

static void gen_address(Node node, int depth) {
    int depth_initial = depth;

    switch (node->kind) {
        case ND_LOCAL_VAR:
            print(depth, "# gen_address: ND_LOCAL_VAR");
            print(depth, "mov rax, rbp");
            print(depth, "sub rax, %d", node->item->offset);
            print(depth++, "push rax");
            break;

        case ND_DEREF:
            print(depth, "# gen_address: ND_DEREF");
            gen(node_get_child(node, 0), depth++);
            break;

        default:
            error("アドレスが計算できません");
    }

    if (depth != depth_initial + 1) error("gen_address: unreachable");
}

static void gen(Node node, int depth) {
    int depth_initial = depth;
    int id;

    switch (node->kind) {
        case ND_CALL:
            print(depth, "# ND_CALL");

            if (node->children->len > 6)
                error("6 個以上の実引数には対応していません");

            for (int i = node->children->len - 1; i >= 0; i--) {
                gen(node_get_child(node, i), depth++);

                switch (i) {
                    case 0:
                        print(depth--, "pop rdi");
                        break;
                    case 1:
                        print(depth--, "pop rsi");
                        break;
                    case 2:
                        print(depth--, "pop rdx");
                        break;
                    case 3:
                        print(depth--, "pop rcx");
                        break;
                    case 4:
                        print(depth--, "pop r8");
                        break;
                    case 5:
                        print(depth--, "pop r9");
                        break;
                }
            }

            print(depth++, "push %d", node->children->len);
            print(depth--, "pop rax");

            if (depth % 2 == 1) {
                print(depth++, "push rdi");
                print(depth, "call %s", node->item->name);
                print(depth--, "pop rdi");

            } else {
                print(depth, "call %s", node->item->name);
            }

            print(depth++, "push rax");
            break;

        case ND_BLOCK:
            print(depth, "# ND_BLOCK");

            for (int i = 0; i < node->children->len; i++) {
                gen(node_get_child(node, i), depth++);
                print(depth--, "pop rax");
            }
            print(depth++, "push rax");
            break;

        case ND_RETURN:
            print(depth, "# ND_RETURN");

            gen(node_get_child(node, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "mov rsp, rbp");
            depth = 2;
            print(depth--, "pop rbp");
            print(depth, "ret");
            break;

        case ND_CONST_INT:
            print(depth, "# ND_CONST_INT");

            print(depth++, "push %d", node->val_int);
            break;

        case ND_LOCAL_VAR:
            print(depth, "# ND_LOCAL_VAR");

            gen_address(node, depth++);

            print(depth--, "pop rax");
            print(depth, "mov rax, [rax]");
            print(depth++, "push rax");
            break;

        case ND_ASSIGN:
            print(depth, "# ND_ASSIGN");

            gen_address(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");
            print(depth, "mov [rax], rdi");
            print(depth++, "push rdi");
            break;

        case ND_NULL:
            print(depth, "# ND_NULL");
            print(depth++, "push rax");
            break;

        case ND_IF:
            id = gen_id();

            print(depth, "# ND_IF");

            gen(node_get_child(node, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lend%08d", id);

            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rax");
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");
            break;

        case ND_IF_ELSE:
            id = gen_id();

            print(depth, "# ND_IF_ELSE");

            gen(node_get_child(node, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lelse%08d", id);

            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rax");
            print(depth, "jmp .Lend%08d", id);
            print(depth, ".Lelse%08d:", id);

            gen(node_get_child(node, 2), depth++);

            print(depth--, "pop rax");
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");

            break;

        case ND_WHILE:
            id = gen_id();

            print(depth, "# ND_WHILE");

            print(depth, ".Lbegin%08d:", id);

            gen(node_get_child(node, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lend%08d", id);

            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rax");
            print(depth, "jmp .Lbegin%08d", id);
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");

            break;

        case ND_FOR:
            id = gen_id();

            print(depth, "# ND_FOR");

            gen(node_get_child(node, 0), depth++);

            print(depth--, "pop rax");
            print(depth, ".Lbegin%08d:", id);

            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lend%08d", id);

            gen(node_get_child(node, 3), depth++);

            print(depth--, "pop rax");

            gen(node_get_child(node, 2), depth++);

            print(depth--, "pop rax");
            print(depth, "jmp .Lbegin%08d", id);
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");

            break;

        case ND_ADDR:
            print(depth, "# ND_ADDR");

            gen_address(node_get_child(node, 0), depth++);

            break;

        case ND_DEREF:
            print(depth, "# ND_DEREF");

            gen(node_get_child(node, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "mov rax, [rax]");
            print(depth++, "push rax");
            break;

        case ND_ADD:
            print(depth, "# ND_ADD");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "add rax, rdi");
            print(depth++, "push rax");
            break;

        case ND_SUB:
            print(depth, "# ND_SUB");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "sub rax, rdi");
            print(depth++, "push rax");
            break;

        case ND_MUL:
            print(depth, "# ND_MUL");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "imul rax, rdi");
            print(depth++, "push rax");
            break;

        case ND_DIV:
            print(depth, "# ND_DIV");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cqo");
            print(depth, "idiv rdi");
            print(depth++, "push rax");
            break;

        case ND_EQUAL:
            print(depth, "# ND_EQUAL");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "sete al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case ND_NOT_EQUAL:
            print(depth, "# ND_NOT_EQUAL");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setne al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case ND_LESS:
            print(depth, "# ND_LESS");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setl al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case ND_LESS_OR_EQUAL:
            print(depth, "# ND_LESS_OR_EQUAL");

            gen(node_get_child(node, 0), depth++);
            gen(node_get_child(node, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setle al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        default:
            error("gen: unreachable");
    }

    if ((node->kind == ND_RETURN && depth != 1) ||
        (node->kind != ND_RETURN && depth != depth_initial + 1)) {
        error("gen: unreachable");
    }
}

static void gen_func(Node node) {
    if (node->kind != ND_FUNC) error("gen_func: unreachable");

    int depth = 0;

    print(depth++, "%s: # ND_FUNC", node->item->name);

    print(depth++, "push rbp");

    print(depth, "mov rbp, rsp");

    print(depth, "sub rsp, %d", node->item->size);
    depth += node->item->size / 8;

    if (node->children->len > 7) error("6 個以上の仮引数には対応していません");

    for (int i = 0; i < node->children->len - 1; i++) {
        gen_address(node_get_child(node, i), depth++);

        print(depth--, "pop rax");

        switch (i) {
            case 0:
                print(depth, "mov [rax], rdi");
                break;
            case 1:
                print(depth, "mov [rax], rsi");
                break;
            case 2:
                print(depth, "mov [rax], rdx");
                break;
            case 3:
                print(depth, "mov [rax], rcx");
                break;
            case 4:
                print(depth, "mov [rax], r8");
                break;
            case 5:
                print(depth, "mov [rax], r9");
                break;
        }
    }

    gen(node_get_child(node, node->children->len - 1), depth++);

    print(depth--, "pop rax");
    print(depth, "mov rsp, rbp");
    depth = 2;
    print(depth--, "pop rbp");
    print(depth, "ret");
}

void gen_program(Node node, FILE *out_fp) {
    out = out_fp;
    if (node->kind != ND_PROGRAM) error("gen_program: unreachable");

    print(0, ".intel_syntax noprefix");
    print(0, ".globl main");

    for (int i = 0; i < node->children->len; i++) {
        fputc('\n', out);
        gen_func(node_get_child(node, i));
    }
}
