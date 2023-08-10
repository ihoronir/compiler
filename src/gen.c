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

static void mov_mem_to_reg(int depth, Type type, RegKind dst_reg,
                           const char *source1, const char *source2) {
    switch (type->kind) {
        case TY_INT:
        case TY_PTR:
            print(depth, "mov %s, %s[%s]", type_reg_name(dst_reg, type),
                  source1, source2);
            break;

        case TY_CHAR:
            // movsx は int のサイズ (32bit) に符号付き拡張するので
            // dst には int が入るレジスタを指定する
            print(depth, "movsx %s, BYTE PTR %s[%s]",
                  type_reg_name(dst_reg, new_type_int()), source1, source2);
            break;

        default:
            assert(0);
    }
}

static void gen_typed_expr(TypedExpr typed_expr, int depth);

static void gen_address(TypedExpr typed_expr, int depth) {
    int depth_initial = depth;

    switch (typed_expr->kind) {
        case EXP_LOCAL_VAR:
            print(depth, "# gen_address: EXP_LOCAL_VAR");
            print(depth, "mov rax, rbp");
            print(depth, "sub rax, %d", typed_expr->item->offset);
            print(depth++, "push rax");
            break;

        case EXP_GLOBAL_VAR:
            print(depth, "# gen_address: EXP_GLOBAL_VAR");
            print(depth, "lea rax, [rip + %s]", typed_expr->item->name);
            print(depth++, "push rax");
            break;

        case EXP_DEREF:
            print(depth, "# gen_address: EXP_DEREF");
            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            break;

        case EXP_STRING:
            // gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            print(depth, "# gen_address: EXP_STRING");
            print(depth, "lea rax, [rip + .LC%d]",
                  typed_expr->string_item->label);
            print(depth++, "push rax");
            break;

        default:
            error("アドレスが計算できません");
    }

    if (depth != depth_initial + 1) error("gen_address: depth が間違ってます");
}

static void gen_typed_expr(TypedExpr typed_expr, int depth) {
    int depth_initial = depth;

    if (typed_expr->type->kind == TY_ARR) {
        gen_address(typed_expr, depth);
        return;
    }

    switch (typed_expr->kind) {
        case EXP_SIZEOF:
            assert(0);  // ここに来る前に整数に置き換えられている

        case EXP_STRING:
            assert(0);  // 冒頭の if により gen_address で対処される。
                        //
        case EXP_COMPOUND_ADD: {
            TypedExpr dst = typed_expr_get_child(typed_expr, 0);
            TypedExpr src = typed_expr_get_child(typed_expr, 1);

            gen_address(dst, depth++);  // スタックトップには &dst
            print(depth--, "pop rax");
            print(depth++, "push rax");  // スタックトップと rax に &dst
            print(depth, "mov rax, [rax]");  // rax に dst
            print(depth++, "push rax");  // スタックに下が &dst 上が dst
            gen_typed_expr(src, depth++);  // スタックに src をつむ
                                           //
                                           // src
                                           // dst
                                           // &dst
                                           //
            print(depth--, "pop rax");     //
            print(depth--, "pop rdi");
            print(depth,
                  "add rax, rdi");  // rax に src + dst 。スタックトップに &dst
            print(depth--, "pop rdi");  // rdi に&dst スタック空
            print(depth, "mov [rdi], %s",
                  type_reg_name(REG_RAX, dst->type));  //
            print(depth++, "push rax");
        } break;

        case EXP_FUNC:
            error("関数名を値としてはつかえません");

        case EXP_DECAY:
            print(depth, "# EXP_DECAY");
            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            break;

        case EXP_CALL:
            print(depth, "# EXP_CALL");

            if (typed_expr->children->len > 7)
                error("6 個以上の実引数には対応していません");

            for (int i = typed_expr->children->len - 1; i >= 1; i--) {
                gen_typed_expr(typed_expr_get_child(typed_expr, i), depth++);

                switch (i) {
                    case 1:
                        print(depth--, "pop rdi");
                        break;
                    case 2:
                        print(depth--, "pop rsi");
                        break;
                    case 3:
                        print(depth--, "pop rdx");
                        break;
                    case 4:
                        print(depth--, "pop rcx");
                        break;
                    case 5:
                        print(depth--, "pop r8");
                        break;
                    case 6:
                        print(depth--, "pop r9");
                        break;
                }
            }

            print(depth++, "push %d", typed_expr->children->len);
            print(depth--, "pop rax");

            TypedExpr func = typed_expr_get_child(typed_expr, 0);
            assert(func->kind == EXP_FUNC);
            char *name = func->item->name;

            if (depth % 2 == 1) {
                print(depth++, "add rsp, 8");
                print(depth, "call %s", name);
                print(depth--, "sub rsp, 8");

            } else {
                print(depth, "call %s", name);
            }

            print(depth++, "push rax");
            break;

        case EXP_CONST_INT:
            print(depth, "# EXP_CONST_INT");

            print(depth++, "push %d", typed_expr->val_int);
            break;

        case EXP_LOCAL_VAR:
            print(depth, "# EXP_LOCAL_VAR");

            gen_address(typed_expr, depth++);

            print(depth--, "pop rax");
            mov_mem_to_reg(depth, typed_expr->type, REG_RAX, "", "rax");
            print(depth++, "push rax");
            break;

        case EXP_GLOBAL_VAR:
            print(depth, "# EXP_GLOBAL_VAR");

            mov_mem_to_reg(depth, typed_expr->type, REG_RAX,
                           typed_expr->item->name, "rip");
            print(depth++, "push rax");
            break;

        case EXP_ASSIGN:
            print(depth, "# EXP_ASSIGN");

            TypedExpr dst = typed_expr_get_child(typed_expr, 0);
            TypedExpr src = typed_expr_get_child(typed_expr, 1);
            gen_address(dst, depth++);
            gen_typed_expr(src, depth++);

            char *src_reg_name = type_reg_name(REG_RDI, dst->type);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");
            print(depth, "mov [rax], %s", src_reg_name);
            print(depth++, "push rdi");
            break;

        case EXP_ADDR:
            print(depth, "# EXP_ADDR");

            gen_address(typed_expr_get_child(typed_expr, 0), depth++);

            break;

        case EXP_DEREF:
            print(depth, "# EXP_DEREF");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "mov rax, [rax]");
            // TODO: mov_mem_to_reg を使う
            print(depth++, "push rax");
            break;

        case EXP_ADD: {
            print(depth, "# EXP_ADD");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "add rax, rdi");
            print(depth++, "push rax");
        } break;

        case EXP_SUB:
            print(depth, "# EXP_SUB");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "sub rax, rdi");
            print(depth++, "push rax");
            break;

        case EXP_MUL:
            print(depth, "# EXP_MUL");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "imul rax, rdi");
            print(depth++, "push rax");
            break;

        case EXP_DIV:
            print(depth, "# EXP_DIV");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cqo");
            print(depth, "idiv rdi");
            print(depth++, "push rax");
            break;

        case EXP_EQUAL:
            print(depth, "# EXP_EQUAL");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "sete al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case EXP_NOT_EQUAL:
            print(depth, "# EXP_NOT_EQUAL");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setne al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case EXP_LESS:
            print(depth, "# EXP_LESS");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

            print(depth--, "pop rdi");
            print(depth--, "pop rax");

            print(depth, "cmp rax, rdi");
            print(depth, "setl al");
            print(depth, "movzb rax, al");
            print(depth++, "push rax");
            break;

        case EXP_LESS_OR_EQUAL:
            print(depth, "# EXP_LESS_OR_EQUAL");

            gen_typed_expr(typed_expr_get_child(typed_expr, 0), depth++);
            gen_typed_expr(typed_expr_get_child(typed_expr, 1), depth++);

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
            gen_typed_expr(stmt_get_typed_expr_child(stmt, 0), depth++);

            break;

        case STMT_RETURN:
            print(depth, "# STMT_RETURN");

            gen_typed_expr(stmt_get_typed_expr_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, "mov rsp, rbp");
            depth = 2;
            print(depth--, "pop rbp");
            print(depth, "ret");
            break;

        case STMT_IF: {
            int id = gen_id();

            print(depth, "# STMT_IF");

            gen_typed_expr(stmt_get_typed_expr_child(stmt, 0), depth++);

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

            gen_typed_expr(stmt_get_typed_expr_child(stmt, 0), depth++);

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

            gen_typed_expr(stmt_get_typed_expr_child(stmt, 0), depth++);

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

            gen_typed_expr(stmt_get_typed_expr_child(stmt, 0), depth++);

            print(depth--, "pop rax");
            print(depth, ".Lbegin%08d:", id);

            gen_typed_expr(stmt_get_typed_expr_child(stmt, 1), depth++);

            print(depth--, "pop rax");
            print(depth, "cmp rax, 0");
            print(depth, "je .Lend%08d", id);

            gen_stmt(stmt_get_stmt_child(stmt, 0), depth++);

            print(depth--, "pop rax");

            gen_typed_expr(stmt_get_typed_expr_child(stmt, 2), depth++);

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

static void gen_toplevel_definition(ToplevelDefinition tld) {
    int depth = 0;

    switch (tld->kind) {
        case TLD_FUNC_DEF: {
            print(depth, ".text");
            print(depth++, "%s: # TOPLEVEL_DEFINITION", tld->item->name);

            print(depth++, "push rbp");

            print(depth, "mov rbp, rsp");

            print(depth, "sub rsp, %d", tld->item->size);
            depth += tld->item->size / 8;

            if (tld->untyped_expr_children->len > 6)
                error("7 個以上の仮引数には対応していません");

            for (int i = 0; i < tld->untyped_expr_children->len; i++) {
                TypedExpr param = tld->typed_expr_children->buf[i];
                gen_address(param, depth++);

                print(depth--, "pop rax");

                switch (i) {
                    case 0:
                        print(depth, "mov [rax], %s",
                              type_reg_name(REG_RDI, param->type));
                        break;
                    case 1:
                        print(depth, "mov [rax], %s",
                              type_reg_name(REG_RSI, param->type));
                        break;
                    case 2:
                        print(depth, "mov [rax], %s",
                              type_reg_name(REG_RDX, param->type));
                        break;
                    case 3:
                        print(depth, "mov [rax], %s",
                              type_reg_name(REG_RCX, param->type));
                        break;
                    case 4:
                        print(depth, "mov [rax], %s",
                              type_reg_name(REG_R8, param->type));
                        break;
                    case 5:
                        print(depth, "mov [rax], %s",
                              type_reg_name(REG_R9, param->type));
                        break;
                }
            }

            for (int i = 0; i < tld->stmt_children->len; i++) {
                gen_stmt(tld->stmt_children->buf[i], depth++);
                print(depth--, "pop rax");
            }

            print(depth, "mov rsp, rbp");
            depth = 2;
            print(depth--, "pop rbp");
            print(depth, "ret");
        } break;

        case TLD_GLOBAL_VAR_DEF: {
            print(depth, ".data");
            print(depth++, "%s: # TOPLEVEL_DEFINITION", tld->item->name);
            print(depth++, ".zero %d", type_size(tld->item->type));
        }
    }
}

void gen_program(Vec /* <ToplevelDefinition> */ program, FILE *out_fp) {
    out = out_fp;

    print(0, ".intel_syntax noprefix");
    print(0, ".globl main");

    for (int i = 0; i < string_storage->len; i++) {
        StringItem string_item = string_storage->buf[i];
        fputc('\n', out);
        print(0, ".LC%d:", string_item->label);
        print(1, ".string \"%s\"", string_item->str);
    }

    for (int i = 0; i < program->len; i++) {
        fputc('\n', out);
        ToplevelDefinition child = program->buf[i];
        gen_toplevel_definition(child);
    }
}
