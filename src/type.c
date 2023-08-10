#include <assert.h>

#include "compiler.h"

char *reg[16][4] = {
    {"al", "ax", "eax", "rax"},      {"dil", "di", "edi", "rdi"},
    {"sil", "si", "esi", "rsi"},     {"dl", "dx", "edx", "rdx"},
    {"cl", "cx", "ecx", "rcx"},      {"bpl", "bp", "ebp", "rbp"},
    {"spl", "sp", "esp", "rsp"},     {"bl", "bx", "ebx", "rbx"},
    {"r8b", "r8w", "r8d", "r8"},     {"r9b", "r9w", "r9d", "r9"},
    {"r10b", "r10w", "r10d", "r10"}, {"r11b", "r11w", "r11d", "r11"},
    {"r12b", "r12w", "r12d", "r12"}, {"r13b", "r13w", "r13d", "r13"},
    {"r14b", "r14w", "r14d", "r14"}, {"r15b", "r15w", "r15d", "r15"},
};

char *reg_name(int reg_kind, int size) {
    switch (size) {
        case 1:
            return reg[reg_kind][0];
        case 2:
            return reg[reg_kind][1];
        case 4:
            return reg[reg_kind][2];
        case 8:
            return reg[reg_kind][3];
        default:
            error("非対応のデータサイズです");
    }
}

int type_size(Type type) {
    switch (type->kind) {
        case TY_INT:
            return 4;
        case TY_CHAR:
            return 1;
        case TY_PTR:
            return 8;
        case TY_ARR:
            return type_size(type->ptr_to) * type->arr_len;
        default:
            error("unimplemented");
    }
}

char *type_reg_name(RegKind reg_kind, Type type) {
    return reg_name(reg_kind, type_size(type));
}

char *type_mov_cmd(Type type) {
    switch (type->kind) {
        case TY_INT:
        case TY_PTR:
            return "mov";

        case TY_CHAR:
            return "movsx";

        case TY_FUNC:
        case TY_ARR:
            assert(0);
    }

    assert(0);
}

Type new_type_int() {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_INT;
    return type;
}

Type new_type_char() {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_CHAR;
    return type;
}

Type new_type_ptr(Type ptr_to) {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_PTR;
    type->ptr_to = ptr_to;
    return type;
}

Type new_type_arr(Type arr_of, int arr_len) {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_ARR;
    type->ptr_to = arr_of;
    type->arr_len = arr_len;
    return type;
}

Type new_type_func(Type returning) {
    Type type = checked_malloc(sizeof(*type));
    type->kind = TY_FUNC;
    type->returning = returning;
    return type;
}

int type_is_equal(Type type1, Type type2) {
    if (type1->kind != type2->kind) return 0;
    switch (type1->kind) {
        case TY_INT:
            return 1;
        case TY_PTR:
            return type_is_equal(type1->ptr_to, type2->ptr_to);
        default:
            error("type_is_equal: unimplemented");
            error("type_is_equal: unreacnable");
            return 0;
    }
}

int type_is_compatible(Type type1, Type type2) {
    if (type_is_equal(type1, type2)) return 1;
    if (type1->kind == TY_INT && type2->kind == TY_CHAR) return 1;
    if (type1->kind == TY_CHAR && type2->kind == TY_INT) return 1;

    return 0;
}

int type_is_integer(Type type) {
    switch (type->kind) {
        case TY_INT:
        case TY_CHAR:
            return 1;

        case TY_FUNC:
        case TY_ARR:
        case TY_PTR:
            return 0;
    }
}
