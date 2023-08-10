struct A {
    char a;
    int b;
    char c;
};
int main() {
    struct A a;
    a.c = 74;
    struct A b = a;
    return b.c;
}
