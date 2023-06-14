#include "compiler.h"

#define VEC_DEFAULT_CAPACITY 100

Vec new_vec_with_capacity(int capacity) {
    Vec vec = checkd_malloc(sizeof(*vec));
    vec->len = 0;
    vec->capacity = capacity;
    vec->buf = checkd_malloc(sizeof(void *) * vec->capacity);
    return vec;
}

Vec new_vec() { return new_vec_with_capacity(VEC_DEFAULT_CAPACITY); }

void vec_push(Vec vec, void *ptr) {
    if (vec->len == vec->capacity) {
        vec->capacity *= 2;
        vec->buf = checkd_realloc(vec->buf, vec->capacity);
    }
    vec->buf[vec->len++] = ptr;
}

// void **vec_get(Vec vec, int index) {
//     if (index >= vec->len) error("vec_get: 範囲外アクセス");
//     return &vec->buf[index];
// }
