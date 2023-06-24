#include "compiler.h"

#define STRING_DEFAULT_CAPACITY 50

String new_string_with_capacity(int capacity) {
    String string = checked_malloc(sizeof(*string));
    string->len = 0;
    string->capacity = capacity;
    string->buf = checked_malloc(sizeof(char) * string->capacity);
    return string;
}

String new_string() {
    return new_string_with_capacity(STRING_DEFAULT_CAPACITY);
}

void string_push(String string, char c) {
    if (string->len == string->capacity) {
        string->capacity *= 2;
        string->buf = checked_realloc(string->buf, string->capacity);
    }
    string->buf[string->len++] = c;
}
