#include "compiler.h"

#define LOCAL_VARS_BUF_INIT_CAPACITY 100

typedef struct LocalVar LocalVar;

struct LocalVar {
    char *str;
    int offset;
};

static LocalVar *local_vars_buf;
static int local_vars_buf_len;
static int local_vars_buf_capacity;
static int local_vars_buf_offset_sum;

void init_local_vars_buf() {
    local_vars_buf =
        checkd_malloc(sizeof(LocalVar) * LOCAL_VARS_BUF_INIT_CAPACITY);
    local_vars_buf_len = 0;
    local_vars_buf_capacity = LOCAL_VARS_BUF_INIT_CAPACITY;
    local_vars_buf_offset_sum = 0;
}

static void local_vars_buf_push(char *str) {
    LocalVar local_var;
    local_var.str = str;
    local_var.offset = (local_vars_buf_offset_sum += 8);

    if (local_vars_buf_len == local_vars_buf_capacity) {
        local_vars_buf_capacity *= 2;
        local_vars_buf = checkd_realloc(
            local_vars_buf, sizeof(Token) * local_vars_buf_capacity * 2);
    }
    local_vars_buf[local_vars_buf_len++] = local_var;
}

int get_offset(char *str) {
    int i;
    for (i = 0; i < local_vars_buf_len; i++) {
        if (!strcmp(local_vars_buf[i].str, str))
            return local_vars_buf[i].offset;
    }
    local_vars_buf_push(str);
    return local_vars_buf[local_vars_buf_len - 1].offset;
}
