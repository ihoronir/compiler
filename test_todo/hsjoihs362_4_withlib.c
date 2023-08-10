struct INT_CHARS_INT {
    int a;
    char c[100];
    int b;
};
struct INT_CHARS_INT merge7();
int main() {
    struct INT_CHARS_INT st = merge7(1, 2, 3, 4, 5, 6, 7);
    return st.b - st.a;
}
