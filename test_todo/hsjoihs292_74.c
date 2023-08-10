struct A {
    char a;
    int b;
};
int main() {
    struct A a;
    a.a = 74;
    struct A b;
    b = a;
    return b.a;
}
