struct A {
    int a;
    int b;
};
int main() {
    struct A a;
    a.a = 174;
    struct A b = a;
    return b.a;
}
