#include "compiler.h"

void error(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_FAILURE);
}

void error_at(int line, int row, char *msg) {
    fprintf(stderr, "(%d, %d) %s\n", line, row, msg);
    exit(EXIT_FAILURE);
}

void *checked_malloc(unsigned long len) {
    void *ptr = malloc(len);
    if (ptr == NULL) error("malloc failed");
    return ptr;
}

void *checked_realloc(void *ptr, unsigned long len) {
    void *new_ptr = realloc(ptr, len);
    if (new_ptr == NULL) error("realloc failed");
    return new_ptr;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    // トークナイズ
    char *user_input = argv[1];
    tokenize(user_input);

    // 解析木作成 -> コード生成
    gen_program(program());

    return EXIT_SUCCESS;
}
