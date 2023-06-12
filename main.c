#include <stdlib.h>

#include "compiler.h"

void error(char *msg) {
    fprintf(stderr, "%s\n", msg);
    exit(EXIT_SUCCESS);
}

void error_at(int line, int row, char *msg) {
    printf("(%d, %d) %s\n", line, row, msg);
    exit(EXIT_FAILURE);
}

void *checkd_malloc(unsigned long len) {
    void *ptr = malloc(len);
    if (ptr == NULL) error("malloc failed");
    return ptr;
}

void *checkd_realloc(void *ptr, unsigned long len) {
    void *new_ptr = realloc(ptr, len);
    if (new_ptr == NULL) error("realloc failed");
    return new_ptr;
}

int main(int argc, char **argv) {
    if (argc != 2) {
        error("引数の個数が正しくありません");
        return 1;
    }

    // トークナイズしてパースする
    char *user_input = argv[1];
    tokenize(user_input);
    ast_init();

    // アセンブリの前半部分を出力
    printf(".intel_syntax noprefix\n");
    printf(".globl main\n");
    printf("main:\n");

    // 抽象構文木を下りながらコード生成
    ast_compile();

    // スタックトップに式全体の値が残っているはずなので
    // それをRAXにロードして関数からの返り値とする
    printf("  pop rax\n");
    printf("  ret\n");
    return 0;
}
