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

static void gen_untyped_expr(UntypedExpr untyped_expr, int depth);

static void gen_address(UntypedExpr node, int depth) {
    int depth_initial = depth;

    switch (node->kind) {
        case EXP_LOCAL_VAR:
            print(depth, "# gen_address: EXP_LOCAL_VAR");
            print(depth, "mov rax, rbp");
            print(depth, "sub rax, %d", node->item->offset);
            print(depth++, "push rax");
            break;

        case EXP_DEREF:
            print(depth, "# gen_address: EXP_DEREF");
            gen_untyped_expr(untyped_expr_get_child(node, 0), depth++);
            break;

        default:
            error("アドレスが計算できません");
    }

    if (depth != depth_initial + 1) error("gen_address: unreachable");
}

static void gen_untyped_expr(UntypedExpr untyped_expr, int depth) {
    int depth_initial = depth;

    switch (untyped_expr->kind) {
        case EXP_CALL:
            print(depth, "# EXP_CALL");

            if (untyped_expr->children->len > 6)
                error("6 個以上の実引数には対応していません");

            for (int i = untyped_expr->children->len - 1; i >= 0; i--) {
                gen_untyped_expr(untyped_expr_get_child(untyped_expr, i),
                                 depth++);

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

            print(depth++, "push %d", untyped_expr->children->len);
            print(depth--, "pop rax");

            if (depth % 2 == 1) {
                print(depth++, "add rsp, 8");
                print(depth, "call %s", untyped_expr->item->name);
                print(depth--, "sub rsp, 8");

            } else {
                print(depth, "call %s", untyped_expr->item->name);
            }

            print(depth++, "push rax");
            break;

        case EXP_CONST_INT:
            print(depth, "# EXP_CONST_INT");

            print(depth++, "push %d", untyped_expr->val_int);
            break;

        case EXP_LOCAL_VAR:
            print(depth, "# EXP_LOCAL_VAR");

            gen_address(untyped_expr, depth++);

            print(depth--, "pop rax");
            print(depth, "mov rax, [rax]");
            print(depth++, "push rax");
            break;

        case EXP_ASSIGN:
            print(depth, "# EXP_ASSIGN");

            gen_address(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");
            // print(depth, "QWORD PTR mov [rax], rdi");
            print(depth, "mov [rax], rdi");
            print(depth++, "push rdi");
            break;

        case EXP_ADDR:
            print(depth, "# EXP_ADDR");

            gen_address(untyped_expr_get_child(untyped_expr, 0), depth++);

            break;

        case EXP_DEREF:
            print(depth, "# EXP_DEREF");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "mov rax, [rax]");
            print(depth++, "push rax");
            break;

        case EXP_ADD: {
            print(depth, "# EXP_ADD");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "add rax, rdi");
            print(depth++, "push rax");
        } break;

        case EXP_SUB:
            print(depth, "# EXP_SUB");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "sub rax, rdi");
            print(depth++, "push rax");
            break;

        case EXP_MUL:
            print(depth, "# EXP_MUL");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "imul rax, rdi");
            print(depth++, "push rax");
            break;

        case EXP_DIV:
            print(depth, "# EXP_DIV");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cqo");
            print(depth, "idiv rdi");
            print(depth++, "push rax");
            break;

        case EXP_EQUAL:
            print(depth, "# EXP_EQUAL");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "sete al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case EXP_NOT_EQUAL:
            print(depth, "# EXP_NOT_EQUAL");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setne al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case EXP_LESS:
            print(depth, "# EXP_LESS");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setl al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case EXP_LESS_OR_EQUAL:
            print(depth, "# EXP_LESS_OR_EQUAL");

            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 0), depth++);
            gen_untyped_expr(untyped_expr_get_child(untyped_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setle al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;
    }

    if (depth != depth_initial + 1) assert(0);
}

static void gen_stmt(Stmt stmt, int depth) {
    int depth_initial = depth;

    switch (stmt->kind) {
        case STMT_BLOCK:
            print(depth, "# STMT_BLOCK");

            for (int i = 0; i < stmt->stmt_children->len; i++) {
                gen_stmt(stmt_get_stmt_child(stmt, i), depth++);
                print(depth--, "pop rax");
            }
            print(depth++, "push rax");
            break;

        case STMT_ONLY_EXPR:
            print(depth, "# STMT_ONLY_EXPR");
            gen_untyped_expr(stmt->untyped_expr, depth++);

            break;

        case STMT_RETURN:
            print(depth, "# STMT_RETURN");

            gen_untyped_expr(stmt->untyped_expr, depth++);

            print(depth--, "pop rax");
            print(depth, "mov rsp, rbp");
            depth = 2;
            print(depth--, "pop rbp");
            print(depth, "ret");
            break;

        case STMT_IF: {
            int id = gen_id();

            print(depth, "# STMT_IF");

            gen_untyped_expr(stmt_get_untyped_expr_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lend%08d", id);

            gen_stmt(stmt_get_stmt_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");
        } break;

        case STMT_IF_ELSE: {
            int id = gen_id();

            print(depth, "# STMT_IF_ELSE");

            gen_untyped_expr(stmt_get_untyped_expr_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lelse%08d", id);

            gen_stmt(stmt_get_stmt_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "jmp .Lend%08d", id);
            print(depth, ".Lelse%08d:", id);

            gen_stmt(stmt_get_stmt_child(stmt, 1), depth++);

            print(depth--, "pop rax");
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");

        } break;

        case STMT_WHILE: {
            int id = gen_id();

            print(depth, "# STMT_WHILE");

            print(depth, ".Lbegin%08d:", id);

            gen_untyped_expr(stmt_get_untyped_expr_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lend%08d", id);

            gen_stmt(stmt_get_stmt_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "jmp .Lbegin%08d", id);
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");

        } break;

        case STMT_FOR: {
            int id = gen_id();

            print(depth, "# STMT_FOR");

            gen_untyped_expr(stmt_get_untyped_expr_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, ".Lbegin%08d:", id);

            gen_untyped_expr(stmt_get_untyped_expr_child(stmt, 1), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lend%08d", id);

            gen_stmt(stmt_get_stmt_child(stmt, 0), depth++);

            print(depth--, "pop rax");

            gen_untyped_expr(stmt_get_untyped_expr_child(stmt, 2), depth++);

            print(depth--, "pop rax");
            print(depth, "jmp .Lbegin%08d", id);
            print(depth, ".Lend%08d:", id);
            print(depth++, "push rax");

        } break;

        case STMT_FUNC_DEFINITION:
        case STMT_PROGRAM:
            assert(0);
            break;
    }

    if ((stmt->kind == STMT_RETURN && depth != 1) ||
        (stmt->kind != STMT_RETURN && depth != depth_initial + 1)) {
        assert(0);
    }
}

static void gen_func(Stmt stmt) {
    assert(stmt->kind == STMT_FUNC_DEFINITION);

    int depth = 0;

    print(depth++, "%s: # STMT_FUNC_DEFINITION", stmt->item->name);

    print(depth++, "push rbp");

    print(depth, "mov rbp, rsp");

    print(depth, "sub rsp, %d", stmt->item->size);
    depth += stmt->item->size / 8;

    if (stmt->untyped_expr_children->len > 6)
        error("7 個以上の仮引数には対応していません");

    for (int i = 0; i < stmt->untyped_expr_children->len; i++) {
        gen_address(stmt_get_untyped_expr_child(stmt, i), depth++);

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

    for (int i = 0; i < stmt->stmt_children->len; i++) {
        gen_stmt(stmt_get_stmt_child(stmt, i), depth++);
        print(depth--, "pop rax");
    }

    print(depth, "mov rsp, rbp");
    depth = 2;
    print(depth--, "pop rbp");
    print(depth, "ret");
}

void gen_program(Stmt stmt, FILE *out_fp) {
    out = out_fp;
    assert(stmt->kind == STMT_PROGRAM);

    print(0, ".intel_syntax noprefix");
    print(0, ".globl main");

    for (int i = 0; i < stmt->stmt_children->len; i++) {
        fputc('\n', out);
        Stmt child = stmt_get_stmt_child(stmt, i);
        assert(child->kind == STMT_FUNC_DEFINITION);
        gen_func(child);
    }
}