#include "compiler.h"

#define VEC_DEFAULT_CAPACITY 100

Vec new_vec_with_capacity(int capacity) {
    Vec vec = checked_malloc(sizeof(*vec));
    vec->len = 0;
    vec->capacity = capacity;
    vec->buf = checked_malloc(sizeof(void *) * vec->capacity);
    return vec;
}

Vec new_vec() { return new_vec_with_capacity(VEC_DEFAULT_CAPACITY); }

void vec_push(Vec vec, void *ptr) {
    if (vec->len == vec->capacity) {
        vec->capacity *= 2;
        vec->buf = checked_realloc(vec->buf, sizeof(void *) * vec->capacity);
    }
    vec->buf[vec->len++] = ptr;
}
