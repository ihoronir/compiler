#include "compiler.h"

void error(char *fmt, ...) {
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fputc('\n', stderr);
    va_end(ap);
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

char *read_file(char *path) {
    FILE *fp = fopen(path, "r");
    if (fp == NULL) error("cannot open %s: %s", path, strerror(errno));

    // 末尾にシーク
    if (fseek(fp, 0, SEEK_END) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    // 現在の場所を size に得る
    int size = ftell(fp);

    // 先頭にシーク
    if (fseek(fp, 0, SEEK_SET) == -1)
        error("%s: fseek: %s", path, strerror(errno));

    char *buf = checked_malloc(sizeof(char) * (size + 2));
    fread(buf, size, 1, fp);

    // ファイルが必ず "\n\0" で終わっているようにする
    if (size == 0 || buf[size - 1] != '\n') buf[size++] = '\n';
    buf[size] = '\0';
    fclose(fp);
    return buf;
}

int main(int argc, char **argv) {
    if (argc != 3) {
        error("引数の個数が正しくありません");
        return 1;
    }

    // トークナイズ
    char *user_input = read_file(argv[1]);
    tokenize(user_input);
    FILE *out_fp = fopen(argv[2], "w");

    // 解析木作成 -> コード生成
    //
    Stmt tree = parse_program();
    gen_program(tree, out_fp);

    fclose(out_fp);
    return EXIT_SUCCESS;
}
